/**
 * Payment Txn
 */
#include "hookapi.h"

// clang-format off
uint8_t txn[305] =
{
/* size,upto */
/*   3,  0 */   0x12U, 0x00U, 0x00U,                                                            /* tt = Payment */
/*   5,  3*/    0x22U, 0x80U, 0x00U, 0x00U, 0x00U,                                              /* flags = tfCanonical */
/*   5,  8 */   0x24U, 0x00U, 0x00U, 0x00U, 0x00U,                                              /* sequence = 0 */
/*   5, 13 */   0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                                              /* dtag, flipped */
/*   6, 18 */   0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,                                       /* first ledger seq */
/*   6, 24 */   0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,                                       /* last ledger seq */
/*  49, 30 */   0x61U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                         /* amount field 9 or 49 bytes */
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99,
/*   9, 79 */   0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,                   /* fee      */
/*  35, 88 */   0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* pubkey   */
/*  22,123 */   0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                           /* src acc  */
/*  22,145 */   0x83U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                           /* dst acc  */
/* 138,167 */                                                                                    /* emit details */
/*   0,305 */
};
// clang-format on

// TX BUILDER
#define FLS_OUT (txn + 20U)
#define LLS_OUT (txn + 26U)
#define DTAG_OUT (txn + 14U)
#define AMOUNT_OUT (txn + 30U)
#define FEE_OUT (txn + 80U)
#define HOOK_ACC (txn + 125U)
#define OTX_ACC (txn + 147U)
#define EMIT_OUT (txn + 167U)

#define SVAR(x) &(x), sizeof(x)

#define NOPE(x)\
    rollback(SBUF(x), __LINE__)

int64_t cbak(uint32_t f)
{
    TRACEVAR(f);
    int64_t meta_slot_no = meta_slot(0);
    if (slot_subfield(meta_slot_no, sfTransactionResult, 1) != 1)
        NOPE("Failed to dump transaction result");

    uint8_t tr;
    if (slot(SVAR(tr), 1) != 1)
        NOPE("Failed to dump transaction result");

    if (tr != 0)
        NOPE("Emitted Transaction Result not tesSUCCESS (0).");

    uint8_t f_buf[4];
    UINT32_TO_BUF(f_buf, f);
    accept(SBUF(f_buf), __LINE__);
    return 0;
}

int64_t hook(uint32_t reserved)
{

    TRACESTR("callback.c: Called.");

    // ACCOUNT: Hook Account
    uint8_t hook_acc[20];
    hook_account(HOOK_ACC, 20);

    // ACCOUNT: To Account
    int64_t ret = util_accid(OTX_ACC, 20, SBUF("rPMh7Pi9ct699iZUTWaytJUoHcJ7cgyziK"));

    // TXN: PREPARE: Init
    etxn_reserve(1);

    // TXN PREPARE: FirstLedgerSequence
    uint32_t fls = (uint32_t)ledger_seq() + 1;
    *((uint32_t *)(FLS_OUT)) = FLIP_ENDIAN_32(fls);

    // TXN PREPARE: LastLedgerSequense
    uint32_t lls = fls + 4;
    *((uint32_t *)(LLS_OUT)) = FLIP_ENDIAN_32(lls);

    // TXN PREPARE: Amount
    uint64_t drops = 1000000;
    uint8_t *b = AMOUNT_OUT + 1;
    *b++ = 0b01000000 + ((drops >> 56) & 0b00111111);
    *b++ = (drops >> 48) & 0xFFU;
    *b++ = (drops >> 40) & 0xFFU;
    *b++ = (drops >> 32) & 0xFFU;
    *b++ = (drops >> 24) & 0xFFU;
    *b++ = (drops >> 16) & 0xFFU;
    *b++ = (drops >> 8) & 0xFFU;
    *b++ = (drops >> 0) & 0xFFU;

    // TXN PREPARE: Dest Tag <- Source Tag
    if (otxn_field(DTAG_OUT, 4, sfSourceTag) == 4)
        *(DTAG_OUT - 1) = 0x2EU;

    // TXN PREPARE: Emit Metadata
    etxn_details(EMIT_OUT, 138U);

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

    // TRACEHEX(txn); // <- final tx blob

    // TXN: Emit/Send Txn
    uint8_t emithash[32];
    int64_t emit_result = emit(SBUF(emithash), SBUF(txn));
    if (emit_result > 0)
    {
        accept(SBUF("callback.c: Tx emitted success."), __LINE__);
    }
    accept(SBUF("callback.c: Tx emitted failure."), __LINE__);

    _g(1, 1);
    // unreachable
    return 0;
}