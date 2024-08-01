import { DOESNT_EXIST, NOT_AN_ARRAY, sfMemos } from 'jshooks-api'

const ASSERT = (x, line) => {
  if (!x) {
    trace('line', line, false)
    rollback(x.toString(), line)
  }
}

const Hook = (arg) => {
  ASSERT(otxn_slot(1) === 1, 1)

  ASSERT(slot_size(1) > 0, 2)

  ASSERT(slot_count(1) === NOT_AN_ARRAY, 3)

  ASSERT(slot_count(0) === DOESNT_EXIST, 4)

  ASSERT(slot_subfield(1, sfMemos, 1) === 1, 5)

  ASSERT(slot_size(1) > 0, 6)

  ASSERT(slot_count(1) === 1, 7)

  accept('', 0)
}

export { Hook }
