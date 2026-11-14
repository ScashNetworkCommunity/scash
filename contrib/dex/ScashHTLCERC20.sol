// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

/**
 * @title ScashHTLCERC20
 * @notice Zero-fee, non-custodial SHA-256 Hashed Timelock Contract for native SCASH/USDT
 *         cross-chain atomic swaps (compatible with Ethereum/Arbitrum ERC-20 & Tron TRC-20 USDT).
 * @dev Uses SHA-256 (`sha256(abi.encodePacked(preimage))`) instead of Keccak-256 so the exact
 *      same 32-byte secret preimage unlocks both Scash Core 2.4.0 (`OP_SHA256`) and USDT!
 *      Includes SafeERC20-compatible low-level call handling because Tether's mainnet ERC-20
 *      USDT `transfer` / `transferFrom` does not return a bool.
 */
contract ScashHTLCERC20 {
    struct LockContract {
        address sender;
        address receiver;
        address tokenContract;
        uint256 amount;
        uint256 antiOptionBond; // Paid to receiver on claim or compensated if maker aborts
        bytes32 hashlock; // SHA-256(preimage) matching Scash Core OP_SHA256
        uint256 timelock; // UNIX timestamp after which sender can refund
        bool withdrawn;
        bool refunded;
        bytes32 preimage;
    }

    mapping(bytes32 => LockContract) public contracts;

    event HTLCERC20New(
        bytes32 indexed contractId,
        address indexed sender,
        address indexed receiver,
        address tokenContract,
        uint256 amount,
        bytes32 hashlock,
        uint256 timelock
    );

    event HTLCERC20Withdraw(bytes32 indexed contractId, bytes32 preimage);
    event HTLCERC20Refund(bytes32 indexed contractId);

    function _safeTransferFrom(address token, address from, address to, uint256 value) internal {
        (bool ok, bytes memory data) = token.call(
            abi.encodeWithSelector(0x23b872dd, from, to, value)
        );
        require(ok && (data.length == 0 || abi.decode(data, (bool))), "USDT transferFrom failed");
    }

    function _safeTransfer(address token, address to, uint256 value) internal {
        (bool ok, bytes memory data) = token.call(
            abi.encodeWithSelector(0xa9059cbb, to, value)
        );
        require(ok && (data.length == 0 || abi.decode(data, (bool))), "USDT transfer failed");
    }

    function newContract(
        address receiver,
        bytes32 hashlock,
        uint256 timelock,
        address tokenContract,
        uint256 amount
    ) external returns (bytes32 contractId) {
        require(receiver != address(0), "Invalid receiver");
        require(amount > 0, "Amount must be > 0");
        require(timelock > block.timestamp, "Timelock must be in future");

        contractId = sha256(
            abi.encodePacked(msg.sender, receiver, tokenContract, amount, hashlock, timelock)
        );
        require(contracts[contractId].sender == address(0), "Contract already exists");

        _safeTransferFrom(tokenContract, msg.sender, address(this), amount);

        contracts[contractId] = LockContract({
            sender: msg.sender,
            receiver: receiver,
            tokenContract: tokenContract,
            amount: amount,
            antiOptionBond: 0,
            hashlock: hashlock,
            timelock: timelock,
            withdrawn: false,
            refunded: false,
            preimage: bytes32(0)
        });

        emit HTLCERC20New(
            contractId,
            msg.sender,
            receiver,
            tokenContract,
            amount,
            hashlock,
            timelock
        );
    }


    /**
     * @notice Locks principal USDT plus an anti-free-option griefing bond (e.g. 200 bps = 2%)
     *         so bots cannot lock counterparty liquidity as a risk-free call option.
     */
    function newBondedContract(
        address receiver,
        bytes32 hashlock,
        uint256 timelock,
        address tokenContract,
        uint256 amount,
        uint256 antiOptionBond
    ) external returns (bytes32 contractId) {
        require(receiver != address(0), "Invalid receiver");
        require(amount > 0, "Amount must be > 0");
        require(timelock > block.timestamp, "Timelock must be in future");

        contractId = sha256(
            abi.encodePacked(msg.sender, receiver, tokenContract, amount, antiOptionBond, hashlock, timelock)
        );
        require(contracts[contractId].sender == address(0), "Contract already exists");

        _safeTransferFrom(tokenContract, msg.sender, address(this), amount + antiOptionBond);

        contracts[contractId] = LockContract({
            sender: msg.sender,
            receiver: receiver,
            tokenContract: tokenContract,
            amount: amount,
            antiOptionBond: antiOptionBond,
            hashlock: hashlock,
            timelock: timelock,
            withdrawn: false,
            refunded: false,
            preimage: bytes32(0)
        });

        emit HTLCERC20New(contractId, msg.sender, receiver, tokenContract, amount, hashlock, timelock);
    }

    function withdraw(bytes32 contractId, bytes32 preimage) external returns (bool) {
        LockContract storage c = contracts[contractId];
        require(c.sender != address(0), "Unknown contract");
        require(!c.withdrawn, "Already withdrawn");
        require(!c.refunded, "Already refunded");
        require(sha256(abi.encodePacked(preimage)) == c.hashlock, "Invalid SHA-256 preimage");

        c.preimage = preimage;
        c.withdrawn = true;
        _safeTransfer(c.tokenContract, c.receiver, c.amount);
        if (c.antiOptionBond > 0) {
            _safeTransfer(c.tokenContract, c.sender, c.antiOptionBond);
        }

        emit HTLCERC20Withdraw(contractId, preimage);
        return true;
    }

    function refund(bytes32 contractId) external returns (bool) {
        LockContract storage c = contracts[contractId];
        require(c.sender != address(0), "Unknown contract");
        require(!c.withdrawn, "Already withdrawn");
        require(!c.refunded, "Already refunded");
        require(block.timestamp >= c.timelock, "Timelock not yet expired");

        c.refunded = true;
        _safeTransfer(c.tokenContract, c.sender, c.amount + c.antiOptionBond);

        emit HTLCERC20Refund(contractId);
        return true;
    }
}
