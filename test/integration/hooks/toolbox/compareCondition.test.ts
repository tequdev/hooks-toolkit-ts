// xrpl
import { Invoke, SetHookFlags, TransactionMetadata } from '@transia/xrpl'
// xrpl-helpers
import {
  XrplIntegrationTestContext,
  setupClient,
  teardownClient,
  serverUrl,
} from '../../../../src/libs/xrpl-helpers'
import {
  Xrpld,
  SetHookParams,
  ExecutionUtility,
  createHookPayload,
  setHooksV3,
  clearAllHooksV3,
  iHookParamEntry,
  iHookParamName,
  iHookParamValue,
  generateHash,
} from '../../../../dist/npm/src'
import { uint64ToHex } from '../../../../dist/npm/src/libs/binary-models'

describe('compare condition', () => {
  let testContext: XrplIntegrationTestContext

  beforeAll(async () => {
    testContext = await setupClient(serverUrl)
    const hook = createHookPayload({
      version: 0,
      createFile: 'compare_condition',
      namespace: 'compare_condition',
      flags: SetHookFlags.hsfOverride,
      hookOnArray: ['Invoke'],
    })
    await setHooksV3({
      client: testContext.client,
      seed: testContext.hook1.seed,
      hooks: [{ Hook: hook }],
    } as SetHookParams)
  })
  afterAll(async () => {
    await clearAllHooksV3({
      client: testContext.client,
      seed: testContext.hook1.seed,
    } as SetHookParams)
    await teardownClient(testContext)
  })

  it('basic hook', async () => {
    // INVOKE IN
    const hookWallet = testContext.hook1
    const aliceWallet = testContext.alice

    const preimage = 'test12345'
    const condition = generateHash(Buffer.from(preimage))
    const otxn1Param1 = new iHookParamEntry(
      new iHookParamName('C'),
      new iHookParamValue(condition, true)
    )
    const builtTx1: Invoke = {
      TransactionType: 'Invoke',
      Account: aliceWallet.classicAddress,
      Destination: hookWallet.classicAddress,
      HookParameters: [otxn1Param1.toXrpl()],
    }
    const result1 = await Xrpld.submit(testContext.client, {
      wallet: aliceWallet,
      tx: builtTx1,
    })
    const hookExecutions1 = await ExecutionUtility.getHookExecutionsFromMeta(
      testContext.client,
      result1.meta as TransactionMetadata
    )
    expect(hookExecutions1.executions[0].HookReturnString).toMatch(
      'compare_condition.c: Condition Added.'
    )

    const fulfillment = Buffer.from(preimage).toString('hex')
    const otxn2Param1 = new iHookParamEntry(
      new iHookParamName('F'),
      new iHookParamValue(fulfillment, true)
    )

    const otxn2Param2 = new iHookParamEntry(
      new iHookParamName('FL'),
      new iHookParamValue(uint64ToHex(BigInt(fulfillment.length / 2)), true)
    )
    const builtTx2: Invoke = {
      TransactionType: 'Invoke',
      Account: aliceWallet.classicAddress,
      Destination: hookWallet.classicAddress,
      HookParameters: [otxn2Param1.toXrpl(), otxn2Param2.toXrpl()],
    }

    const result2 = await Xrpld.submit(testContext.client, {
      wallet: aliceWallet,
      tx: builtTx2,
    })
    const hookExecutions2 = await ExecutionUtility.getHookExecutionsFromMeta(
      testContext.client,
      result2.meta as TransactionMetadata
    )
    expect(hookExecutions2.executions[0].HookReturnString).toMatch(
      'compare_condition.c: Verification Success.'
    )
  })
})
