
#define CPU_FEATURE_FLAGS(decl) \
  decl(CX8,               cx8,               0)  /*  next bits are from cpuid 1 (EDX) */ \
  decl(CMOV,              cmov,              1)  \
  decl(FXSR,              fxsr,              2)  \
  decl(HT,                ht,                3)  \
						   \
  decl(MMX,               mmx,               4)  \
  decl(3DNOW_PREFETCH,    3dnow_prefetch,    5)  /* Processor supports 3dnow prefetch and prefetchw instructions */ \
						   /* may not necessarily support other 3dnow instructions */ \
  decl(SSE,               sse,               6)  \
  decl(SSE2,              sse2,              7)  \
						   \
  decl(SSE3,              sse3,              8 ) /* SSE3 comes from cpuid 1 (ECX) */ \
  decl(SSSE3,             ssse3,             9 ) \
  decl(SSE4A,             sse4a,             10) \
  decl(SSE4_1,            sse4_1,            11) \
						   \
  decl(SSE4_2,            sse4_2,            12) \
  decl(POPCNT,            popcnt,            13) \
  decl(LZCNT,             lzcnt,             14) \
  decl(TSC,               tsc,               15) \
						   \
  decl(TSCINV_BIT,        tscinv_bit,        16) \
  decl(TSCINV,            tscinv,            17) \
  decl(AVX,               avx,               18) \
  decl(AVX2,              avx2,              19) \
						   \
  decl(AES,               aes,               20) \
  decl(ERMS,              erms,              21) /* enhanced 'rep movsb/stosb' instructions */ \
  decl(CLMUL,             clmul,             22) /* carryless multiply for CRC */ \
  decl(BMI1,              bmi1,              23) \
						   \
  decl(BMI2,              bmi2,              24) \
  decl(RTM,               rtm,               25) /* Restricted Transactional Memory instructions */ \
  decl(ADX,               adx,               26) \
  decl(AVX512F,           avx512f,           27) /* AVX 512bit foundation instructions */ \
						   \
  decl(AVX512DQ,          avx512dq,          28) \
  decl(AVX512PF,          avx512pf,          29) \
  decl(AVX512ER,          avx512er,          30) \
  decl(AVX512CD,          avx512cd,          31) \
						   \
  decl(AVX512BW,          avx512bw,          32) /* Byte and word vector instructions */ \
  decl(AVX512VL,          avx512vl,          33) /* EVEX instructions with smaller vector length */ \
  decl(SHA,               sha,               34) /* SHA instructions */ \
  decl(FMA,               fma,               35) /* FMA instructions */ \
						   \
  decl(VZEROUPPER,        vzeroupper,        36) /* Vzeroupper instruction */ \
  decl(AVX512_VPOPCNTDQ,  avx512_vpopcntdq,  37) /* Vector popcount */ \
  decl(AVX512_VPCLMULQDQ, avx512_vpclmulqdq, 38) /* Vector carryless multiplication */ \
  decl(AVX512_VAES,       avx512_vaes,       39) /* Vector AES instruction */ \
						   \
  decl(AVX512_VNNI,       avx512_vnni,       40) /* Vector Neural Network Instructions */ \
  decl(FLUSH,             clflush,           41) /* flush instruction */ \
  decl(FLUSHOPT,          clflushopt,        42) /* flusopth instruction */ \
  decl(CLWB,              clwb,              43) /* clwb instruction */ \
						   \
  decl(AVX512_VBMI2,      avx512_vbmi2,      44) /* VBMI2 shift left double instructions */ \
  decl(AVX512_VBMI,       avx512_vbmi,       45) /* Vector BMI instructions */ \
  decl(HV,                hv,                46) /* Hypervisor instructions */ \
  decl(SERIALIZE,         serialize,         47) /* CPU SERIALIZE */ \
  decl(RDTSCP,            rdtscp,            48) /* RDTSCP instruction */ \
  decl(RDPID,             rdpid,             49) /* RDPID instruction */ \
  decl(FSRM,              fsrm,              50) /* Fast Short REP MOV */ \
  decl(GFNI,              gfni,              51) /* Vector GFNI instructions */ \
  decl(AVX512_BITALG,     avx512_bitalg,     52) /* Vector sub-word popcount and bit gather instructions */\
  decl(F16C,              f16c,              53) /* Half-precision and single precision FP conversion instructions*/ \
  decl(PKU,               pku,               54) /* Protection keys for user-mode pages */ \
  decl(OSPKE,             ospke,             55) /* OS enables protection keys */ \
  decl(CET_IBT,           cet_ibt,           56) /* Control Flow Enforcement - Indirect Branch Tracking */ \
  decl(CET_SS,            cet_ss,            57) /* Control Flow Enforcement - Shadow Stack */ \
  decl(AVX512_IFMA,       avx512_ifma,       58) /* Integer Vector FMA instructions*/ \
  decl(AVX_IFMA,          avx_ifma,          59) /* 256-bit VEX-coded variant of AVX512-IFMA*/ \
  decl(APX_F,             apx_f,             60) /* Intel Advanced Performance Extensions*/ \
  decl(SHA512,            sha512,            61) /* SHA512 instructions*/ \
  decl(AVX512_FP16,       avx512_fp16,       62) /* AVX512 FP16 ISA support*/ \
  decl(AVX10_1,           vx10_1,            63) /* AVX10 512 bit vector ISA Version 1 support*/ \
  decl(AVX10_2,           avx10_2,           64) /* AVX10 512 bit vector ISA Version 2 support*/

