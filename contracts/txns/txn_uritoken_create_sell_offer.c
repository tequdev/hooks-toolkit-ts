/**
 * URITokenCreateSellOffer Txn
 */
#include "hookapi.h"

// ---------------------------------------------------------------------------------------------------------------------------------------------

/**
 * 
 * These functions should be moved into the macro.c file
*/

#define UINT256_TO_BUF(buf_raw, i)\
{\
    unsigned char* buf = (unsigned char*)buf_raw;\
    *(uint64_t*)(buf + 0) = *(uint64_t*)(i +  0);\
    *(uint64_t*)(buf + 8) = *(uint64_t*)(i +  8);\
    *(uint64_t*)(buf + 16) = *(uint64_t*)(i + 16);\
    *(uint64_t*)(buf + 24) = *(uint64_t*)(i + 24);\
}
// ---------------------------------------------------------------------------------------------------------------------------------------------

uint8_t txn[295] =
{
    /* size,upto */
    /*   3,  0  */ 0x12U, 0x00U, 0x30U,                                                             /* tt = URITokenCreateSellOffer */
    /*   5,  3  */ 0x22U, 0x80U, 0x00U, 0x00U, 0x00U,                                               /* flags = tfCanonical */
    /*   5,  8  */ 0x24U, 0x00U, 0x00U, 0x00U, 0x00U,                                               /* sequence = 0 */
    /*   5, 13  */ 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                                               /* dtag, flipped */
    /*   6, 18  */ 0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,                                        /* first ledger seq */
    /*   6, 24  */ 0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,                                        /* last ledger seq */
    /*  49, 30  */ 0x61U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* amount field 9 or 49 bytes */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                          /* cont...  */
                    0x99,                                                                            /* cont...  */
    /*  34, 79  */ 0x50U, 0x24U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /* hash256 = URITokenID  */
    /*   9, 113 */ 0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,                          /* fee      */
                    0x00U,                                                                           /* cont...  */
    /*  35, 122 */ 0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* pubkey   */
    /*  22, 157 */ 0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                           /* src acc  */
    /* 116, 179 */                                                                                  /* emit details */
    /*   0, 295 */
};

// TX BUILDER
#define FLS_OUT (txn + 20U)
#define LLS_OUT (txn + 26U)
#define DTAG_OUT (txn + 14U)
#define INDEXID_OUT (txn + 81U)
#define AMOUNT_OUT (txn + 30U)
#define HOOK_ACC (txn + 159U)
#define FEE_OUT (txn + 114U)
#define EMIT_OUT (txn + 179U)

int64_t hook(uint32_t reserved) {

    TRACESTR("txn_uritoken_create_sell_offer.c: Called.");
    
    // ACCOUNT: Hook Account
    uint8_t hook_acc[SFS_ACCOUNT];
    hook_account(HOOK_ACC, 20);

    uint8_t hashid_buf[32];
    uint8_t hashid_key[2] = { 'I', 'D' };
    if (otxn_param(SBUF(hashid_buf), SBUF(hashid_key)) != 32)
    {
        rollback(SBUF("txn_uritoken_create_sell_offer.c: Invalid Txn Parameter `ID`"), __LINE__);
    }
    UINT256_TO_BUF(INDEXID_OUT, hashid_buf);

    uint8_t amount_buf[48];
    uint8_t amount_key[3] = { 'A', 'M', 'T' };
    int64_t amount_native = otxn_param(SBUF(amount_buf), SBUF(amount_key)) == 8;

    TRACEHEX(amount_buf);

    int64_t amount = *((int64_t*)amount_buf);
    TRACEVAR(amount);
    if (float_compare(amount, 0, COMPARE_LESS | COMPARE_EQUAL) == 1)
        rollback(SBUF("txn_uritoken_create_sell_offer.c: Invalid Txn Parameter `AMT`"), __LINE__);

    // TXN: PREPARE: Init
    etxn_reserve(1);

    // TXN PREPARE: FirstLedgerSequence
    uint32_t fls = (uint32_t)ledger_seq() + 1;
    *((uint32_t*)(FLS_OUT)) = FLIP_ENDIAN(fls);

    // TXN PREPARE: LastLedgerSequense
    uint32_t lls = fls + 4 ;
    *((uint32_t*)(LLS_OUT)) = FLIP_ENDIAN(lls);

    // TXN PREPARE: Amount
    if (amount_native)
    {
        uint64_t drops = float_int(amount, 6, 1);
        uint8_t* b = AMOUNT_OUT + 1;
        *b++ = 0b01000000 + (( drops >> 56 ) & 0b00111111 );
        *b++ = (drops >> 48) & 0xFFU;
        *b++ = (drops >> 40) & 0xFFU;
        *b++ = (drops >> 32) & 0xFFU;
        *b++ = (drops >> 24) & 0xFFU;
        *b++ = (drops >> 16) & 0xFFU;
        *b++ = (drops >>  8) & 0xFFU;
        *b++ = (drops >>  0) & 0xFFU;
    }
    else
        float_sto(AMOUNT_OUT, 49, amount_buf + 8, 20, amount_buf + 28, 20, amount, sfAmount);

    // TXN PREPARE: Dest Tag <- Source Tag
    if (otxn_field(DTAG_OUT, 4, sfSourceTag) == 4)
        *(DTAG_OUT-1) = 0x2EU;

    // TXN PREPARE: Emit Metadata
    etxn_details(EMIT_OUT, 116U);

    // TXN PREPARE: Fee
    {
        int64_t fee = etxn_fee_base(SBUF(txn));
        uint8_t *b = FEE_OUT;
        *b++ = 0b01000000 + ((fee >> 56) & 0b00111111);
        *b++ = (fee >> 48) & 0xFFU;
        *b++ = (fee >> 40) & 0xFFU;
        *b++ = (fee >> 32) & 0xFFU;
        *b++ = (fee >> 24) & 0xFFU;
        *b++ = (fee >> 16) & 0xFFU;
        *b++ = (fee >> 8) & 0xFFU;
        *b++ = (fee >> 0) & 0xFFU;
    }

    TRACEHEX(txn);  // <- final tx blob

    // TXN: Emit/Send Txn
    uint8_t emithash[32];
    int64_t emit_result = emit(SBUF(emithash), SBUF(txn));
    if (emit_result > 0)
    {
        accept(SBUF("txn_uritoken_create_sell_offer.c: Tx emitted success."), __LINE__);
    }
    accept(SBUF("txn_uritoken_create_sell_offer.c: Tx emitted failure."), __LINE__);

    _g(1,1);
    // unreachable
    return 0;
}