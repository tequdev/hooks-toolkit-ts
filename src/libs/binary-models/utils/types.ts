// Types
type Bool = boolean
type UInt8 = number
type UInt32 = number
type UInt64 = bigint
type UInt224 = bigint
type VarString = string
type XFL = number
type Currency = string
type XRPAddress = string
type Model = {
  [key: string]:
    | Bool
    | UInt8
    | UInt32
    | UInt64
    | UInt224
    | VarString
    | XFL
    | Currency
    | XRPAddress
    | Model
    | VarModelArray
}
type VarModelArray = Model[]

export {
  Bool,
  UInt8,
  UInt32,
  UInt64,
  UInt224,
  VarString,
  XFL,
  Currency,
  XRPAddress,
  Model,
  VarModelArray,
}
