# Hydro Thunder slice for the differential harness (dreamcomp/engine/docs/differential-harness.md):
# leaf functions chosen by tools/oracle/pick_slice.py from the emitted unit, each compared with
# Flycast's interpreter (game/oracle-cases.json). Addresses only; the code is read from the owner's
# extracted, git-ignored 1ST_READ.BIN, and nothing is added without it.
set(_ht_image ${CMAKE_CURRENT_LIST_DIR}/extracted/fs/1ST_READ.BIN)
if(EXISTS ${_ht_image})
  list(APPEND DREAM_SH4_PROGRAMS
    "ht_01286c|${_ht_image}|0x0c010000|0x0c01286c:0x0c0128f2:ht_01286c"
    "ht_04879c|${_ht_image}|0x0c010000|0x0c04879c:0x0c0487e2:ht_04879c"
    "ht_095f06|${_ht_image}|0x0c010000|0x0c095f06:0x0c095f5c:ht_095f06"
    "ht_05ecc0|${_ht_image}|0x0c010000|0x0c05ecc0:0x0c05ed2a:ht_05ecc0"
    "ht_075148|${_ht_image}|0x0c010000|0x0c075148:0x0c0751a8:ht_075148"
    "ht_095ed8|${_ht_image}|0x0c010000|0x0c095ed8:0x0c095f06:ht_095ed8"
    "ht_07d65e|${_ht_image}|0x0c010000|0x0c07d65e:0x0c07d692:ht_07d65e"
    "ht_07533e|${_ht_image}|0x0c010000|0x0c07533e:0x0c0753a4:ht_07533e"
    "ht_066de8|${_ht_image}|0x0c010000|0x0c066de8:0x0c066e06:ht_066de8"
    "ht_0a221c|${_ht_image}|0x0c010000|0x0c0a221c:0x0c0a223c:ht_0a221c"
    "ht_0a2f5c|${_ht_image}|0x0c010000|0x0c0a2f5c:0x0c0a2f7c:ht_0a2f5c"
    "ht_050260|${_ht_image}|0x0c010000|0x0c050260:0x0c050284:ht_050260"
    "ht_066ea0|${_ht_image}|0x0c010000|0x0c066ea0:0x0c066eb6:ht_066ea0"
    "ht_0a21fc|${_ht_image}|0x0c010000|0x0c0a21fc:0x0c0a2218:ht_0a21fc"
    "ht_04b052|${_ht_image}|0x0c010000|0x0c04b052:0x0c04b086:ht_04b052"
    "ht_04f1b0|${_ht_image}|0x0c010000|0x0c04f1b0:0x0c04f20e:ht_04f1b0"
    "ht_033f94|${_ht_image}|0x0c010000|0x0c033f94:0x0c034044:ht_033f94"
    "ht_05d5b6|${_ht_image}|0x0c010000|0x0c05d5b6:0x0c05d66c:ht_05d5b6"
    "ht_064b22|${_ht_image}|0x0c010000|0x0c064b22:0x0c064b90:ht_064b22"
    "ht_056512|${_ht_image}|0x0c010000|0x0c056512:0x0c0565b6:ht_056512"
    "ht_050e68|${_ht_image}|0x0c010000|0x0c050e68:0x0c050f0e:ht_050e68"
    "ht_08818c|${_ht_image}|0x0c010000|0x0c08818c:0x0c088214:ht_08818c"
    "ht_05148e|${_ht_image}|0x0c010000|0x0c05148e:0x0c051524:ht_05148e"
    "ht_02b356|${_ht_image}|0x0c010000|0x0c02b356:0x0c02b3d4:ht_02b356"
    "ht_02cdd8|${_ht_image}|0x0c010000|0x0c02cdd8:0x0c02ce56:ht_02cdd8"
    "ht_02ddba|${_ht_image}|0x0c010000|0x0c02ddba:0x0c02de38:ht_02ddba"
    "ht_035086|${_ht_image}|0x0c010000|0x0c035086:0x0c03513c:ht_035086"
    "ht_096ad0|${_ht_image}|0x0c010000|0x0c096ad0:0x0c096b64:ht_096ad0"
    "ht_05efe8|${_ht_image}|0x0c010000|0x0c05efe8:0x0c05f094:ht_05efe8"
    "ht_03e97e|${_ht_image}|0x0c010000|0x0c03e97e:0x0c03ea38:ht_03e97e"
  )
endif()
