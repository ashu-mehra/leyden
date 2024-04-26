/*
 * Copyright (c) 2013, 2024, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef CPU_X86_STUBROUTINES_X86_HPP
#define CPU_X86_STUBROUTINES_X86_HPP

// This file holds the platform specific parts of the StubRoutines
// definition. See stubRoutines.hpp for a description on how to
// extend it.

static bool returns_to_call_stub(address return_pc) { return return_pc == _call_stub_return_address; }

enum platform_dependent_constants {
  // simply increase sizes if too small (assembler will crash if too small)
  _initial_stubs_code_size      = 20000 WINDOWS_ONLY(+1000),
  _continuation_stubs_code_size =  1000 LP64_ONLY(+1000),
  // AVX512 intrinsics add more code in 64-bit VM,
  // Windows have more code to save/restore registers
  _compiler_stubs_code_size     = 20000 LP64_ONLY(+39000) WINDOWS_ONLY(+2000),
  _final_stubs_code_size        = 10000 LP64_ONLY(+20000) WINDOWS_ONLY(+2000) ZGC_ONLY(+20000)
};

class x86 {
 friend class StubGenerator;
 friend class VMStructs;

#ifdef _LP64
 private:
  static address _get_previous_sp_entry;

  static address _f2i_fixup;
  static address _f2l_fixup;
  static address _d2i_fixup;
  static address _d2l_fixup;

  static address _float_sign_mask;
  static address _float_sign_flip;
  static address _double_sign_mask;
  static address _double_sign_flip;
  static address _compress_perm_table32;
  static address _compress_perm_table64;
  static address _expand_perm_table32;
  static address _expand_perm_table64;

 public:

  static address get_previous_sp_entry() {
    return _get_previous_sp_entry;
  }

  static address f2i_fixup() {
    return _f2i_fixup;
  }

  static address f2l_fixup() {
    return _f2l_fixup;
  }

  static address d2i_fixup() {
    return _d2i_fixup;
  }

  static address d2l_fixup() {
    return _d2l_fixup;
  }

  static address float_sign_mask() {
    return _float_sign_mask;
  }

  static address float_sign_flip() {
    return _float_sign_flip;
  }

  static address double_sign_mask() {
    return _double_sign_mask;
  }

  static address double_sign_flip() {
    return _double_sign_flip;
  }

#else // !LP64

 private:
  static address _verify_fpu_cntrl_wrd_entry;
  static address _d2i_wrapper;
  static address _d2l_wrapper;

  static jint    _fpu_cntrl_wrd_std;
  static jint    _fpu_cntrl_wrd_24;
  static jint    _fpu_cntrl_wrd_trunc;

  static jint    _fpu_subnormal_bias1[3];
  static jint    _fpu_subnormal_bias2[3];

 public:
  static address verify_fpu_cntrl_wrd_entry() { return _verify_fpu_cntrl_wrd_entry; }
  static address d2i_wrapper()                { return _d2i_wrapper; }
  static address d2l_wrapper()                { return _d2l_wrapper; }
  static address addr_fpu_cntrl_wrd_std()     { return (address)&_fpu_cntrl_wrd_std;   }
  static address addr_fpu_cntrl_wrd_24()      { return (address)&_fpu_cntrl_wrd_24;    }
  static address addr_fpu_cntrl_wrd_trunc()   { return (address)&_fpu_cntrl_wrd_trunc; }
  static address addr_fpu_subnormal_bias1()   { return (address)&_fpu_subnormal_bias1; }
  static address addr_fpu_subnormal_bias2()   { return (address)&_fpu_subnormal_bias2; }

  static jint    fpu_cntrl_wrd_std()          { return _fpu_cntrl_wrd_std; }
#endif // !LP64

 private:
  static jint    _mxcsr_std;
#ifdef _LP64
  static jint    _mxcsr_rz;
#endif // _LP64

  static address _verify_mxcsr_entry;

  // masks and table for CRC32
  static const uint64_t _crc_by128_masks[];
  static const juint    _crc_table[];
#ifdef _LP64
  static const juint    _crc_by128_masks_avx512[];
  static const juint    _crc_table_avx512[];
  static const juint    _crc32c_table_avx512[];
  static const juint    _shuf_table_crc32_avx512[];
#endif // _LP64
  // table for CRC32C
  static juint* _crc32c_table;
  // table for arrays_hashcode
  static const jint _arrays_hashcode_powers_of_31[];

  // upper word mask for sha1
  static address _upper_word_mask_addr;
  // byte flip mask for sha1
  static address _shuffle_byte_flip_mask_addr;

  //k256 table for sha256
  static const juint _k256[];
  static address _k256_adr;
  static address _vector_short_to_byte_mask;
  static address _vector_float_sign_mask;
  static address _vector_float_sign_flip;
  static address _vector_double_sign_mask;
  static address _vector_double_sign_flip;
  static address _vector_long_sign_mask;
  static address _vector_all_bits_set;
  static address _vector_int_mask_cmp_bits;
  static address _vector_byte_perm_mask;
  static address _vector_int_to_byte_mask;
  static address _vector_int_to_short_mask;
  static address _vector_32_bit_mask;
  static address _vector_64_bit_mask;
  static address _vector_int_shuffle_mask;
  static address _vector_byte_shuffle_mask;
  static address _vector_short_shuffle_mask;
  static address _vector_long_shuffle_mask;
  static address _vector_iota_indices;
  static address _vector_popcount_lut;
  static address _vector_count_leading_zeros_lut;
  static address _vector_reverse_bit_lut;
  static address _vector_reverse_byte_perm_mask_long;
  static address _vector_reverse_byte_perm_mask_int;
  static address _vector_reverse_byte_perm_mask_short;
#ifdef _LP64
  static juint _k256_W[];
  static address _k256_W_adr;
  static const julong _k512_W[];
  static address _k512_W_addr;
  // byte flip mask for sha512
  static address _pshuffle_byte_flip_mask_addr_sha512;
  static address _pshuffle_byte_flip_mask_off32_addr_sha512;
  // Masks for base64
  static address _encoding_table_base64;
  static address _shuffle_base64;
  static address _avx2_shuffle_base64;
  static address _avx2_input_mask_base64;
  static address _avx2_lut_base64;
  static address _avx2_decode_tables_base64;
  static address _avx2_decode_lut_tables_base64;
  static address _lookup_lo_base64;
  static address _lookup_hi_base64;
  static address _lookup_lo_base64url;
  static address _lookup_hi_base64url;
  static address _pack_vec_base64;
  static address _join_0_1_base64;
  static address _join_1_2_base64;
  static address _join_2_3_base64;
  static address _decoding_table_base64;
#endif
  // byte flip mask for sha256
  static address _pshuffle_byte_flip_mask_addr;
  static address _pshuffle_byte_flip_mask_off32_addr;
  static address _pshuffle_byte_flip_mask_off64_addr;

#ifdef _LP64
  // constants moved from stubGenerator_x86_64_aes.cpp
  static const uint64_t _key_shuffle_mask[];
  static const uint64_t _counter_shuffle_mask[];
  static const uint64_t _counter_mask_linc0[];
  static const uint64_t _counter_mask_linc1[];
  static const uint64_t _counter_mask_linc1f[];
  static const uint64_t _counter_mask_linc2[];
  static const uint64_t _counter_mask_linc2f[];
  static const uint64_t _counter_mask_linc4[];
  static const uint64_t _counter_mask_linc8[];
  static const uint64_t _counter_mask_linc16[];
  static const uint64_t _counter_mask_linc32[];
  static const uint64_t _counter_mask_ones[];
  static const uint64_t _ghash_polynomial_reduction[];
  static const uint64_t _ghash_polynomial_two_one[];

  // constants moved from stubGenerator_x86_64_ghash.cpp
  static const uint64_t _ghash_shuffle_mask[];
  static const uint64_t _ghash_long_swap_mask[];
  static const uint64_t _ghash_byte_swap_mask[];
  static const uint64_t _ghash_polynomial[];

  // constants moved from stubGenerator_x86_64_adler.cpp
  static const juint _adler32_ascale_table[];
  static const juint _adler32_shuf0_table[];
  static const juint _adler32_shuf1_table[];

  // constants moved from stubGenerator_x86_64_chacha.cpp
  static const uint64_t _cc20_counter_add_avx[];
  static const uint64_t _cc20_counter_add_avx512[];
  static const uint64_t _cc20_lrot_consts[];

  // constants moved from stubGenerator_x86_64_poly.cpp
  static const uint64_t _poly1305_pad_msg[];
  static const uint64_t _poly1305_mask42[];
  static const uint64_t _poly1305_mask44[];

#endif // _LP64

 public:
  static address addr_mxcsr_std()        { return (address)&_mxcsr_std; }
#ifdef _LP64
  static address addr_mxcsr_rz()        { return (address)&_mxcsr_rz; }
#endif // _LP64
  static address verify_mxcsr_entry()    { return _verify_mxcsr_entry; }
  static address crc_by128_masks_addr()  { return (address)_crc_by128_masks; }
  static address crc_by128_masks_off16_addr()  { return ((address)_crc_by128_masks) + 16; }
  static address crc_by128_masks_off32_addr()  { return ((address)_crc_by128_masks) + 32; }
#ifdef _LP64
  static address crc_by128_masks_avx512_addr()  { return (address)_crc_by128_masks_avx512; }
  static address crc_by128_masks_avx512_off16_addr()  { return ((address)_crc_by128_masks_avx512) + 16; }
  static address crc_by128_masks_avx512_off32_addr()  { return ((address)_crc_by128_masks_avx512) + 32; }
  static address shuf_table_crc32_avx512_addr()  { return (address)_shuf_table_crc32_avx512; }
  static address crc_table_avx512_addr()  { return (address)_crc_table_avx512; }
  static address crc32c_table_avx512_addr()  { return (address)_crc32c_table_avx512; }
#endif // _LP64
  static address upper_word_mask_addr() { return _upper_word_mask_addr; }
  static address shuffle_byte_flip_mask_addr() { return _shuffle_byte_flip_mask_addr; }
  static address k256_addr()      { return _k256_adr; }
  static address method_entry_barrier() { return _method_entry_barrier; }

  static address vector_short_to_byte_mask() {
    return _vector_short_to_byte_mask;
  }
  static address vector_float_sign_mask() {
    return _vector_float_sign_mask;
  }

  static address vector_float_sign_flip() {
    return _vector_float_sign_flip;
  }

  static address vector_double_sign_mask() {
    return _vector_double_sign_mask;
  }

  static address vector_double_sign_flip() {
    return _vector_double_sign_flip;
  }

  static address vector_all_bits_set() {
    return _vector_all_bits_set;
  }

  static address vector_int_mask_cmp_bits() {
    return _vector_int_mask_cmp_bits;
  }

  static address vector_byte_perm_mask() {
    return _vector_byte_perm_mask;
  }

  static address vector_int_to_byte_mask() {
    return _vector_int_to_byte_mask;
  }

  static address vector_int_to_short_mask() {
    return _vector_int_to_short_mask;
  }

  static address vector_32_bit_mask() {
    return _vector_32_bit_mask;
  }

  static address vector_64_bit_mask() {
    return _vector_64_bit_mask;
  }

  static address vector_int_shuffle_mask() {
    return _vector_int_shuffle_mask;
  }

  static address vector_byte_shuffle_mask() {
    return _vector_byte_shuffle_mask;
  }

  static address vector_short_shuffle_mask() {
    return _vector_short_shuffle_mask;
  }

  static address vector_long_shuffle_mask() {
    return _vector_long_shuffle_mask;
  }

  static address vector_long_sign_mask() {
    return _vector_long_sign_mask;
  }

  static address vector_iota_indices() {
    return _vector_iota_indices;
  }

  static address vector_count_leading_zeros_lut() {
    return _vector_count_leading_zeros_lut;
  }

  static address vector_reverse_bit_lut() {
    return _vector_reverse_bit_lut;
  }

  static address vector_reverse_byte_perm_mask_long() {
    return _vector_reverse_byte_perm_mask_long;
  }

  static address vector_reverse_byte_perm_mask_int() {
    return _vector_reverse_byte_perm_mask_int;
  }

  static address vector_reverse_byte_perm_mask_short() {
    return _vector_reverse_byte_perm_mask_short;
  }

  static address vector_popcount_lut() {
    return _vector_popcount_lut;
  }
#ifdef _LP64
  static address k256_W_addr()    { return _k256_W_adr; }
  static address k512_W_addr()    { return _k512_W_addr; }
  static address pshuffle_byte_flip_mask_addr_sha512() { return _pshuffle_byte_flip_mask_addr_sha512; }
  static address pshuffle_byte_flip_mask_off32_addr_sha512() { return _pshuffle_byte_flip_mask_off32_addr_sha512; }
  static address base64_encoding_table_addr() { return _encoding_table_base64; }
  static address base64_shuffle_addr() { return _shuffle_base64; }
  static address base64_avx2_shuffle_addr() { return _avx2_shuffle_base64; }
  static address base64_avx2_input_mask_addr() { return _avx2_input_mask_base64; }
  static address base64_avx2_lut_addr() { return _avx2_lut_base64; }
  static address base64_vbmi_lookup_lo_addr() { return _lookup_lo_base64; }
  static address base64_vbmi_lookup_hi_addr() { return _lookup_hi_base64; }
  static address base64_vbmi_lookup_lo_url_addr() { return _lookup_lo_base64url; }
  static address base64_vbmi_lookup_hi_url_addr() { return _lookup_hi_base64url; }
  static address base64_vbmi_pack_vec_addr() { return _pack_vec_base64; }
  static address base64_vbmi_join_0_1_addr() { return _join_0_1_base64; }
  static address base64_vbmi_join_1_2_addr() { return _join_1_2_base64; }
  static address base64_vbmi_join_2_3_addr() { return _join_2_3_base64; }
  static address base64_decoding_table_addr() { return _decoding_table_base64; }
  static address base64_AVX2_decode_tables_addr() { return _avx2_decode_tables_base64; }
  static address base64_AVX2_decode_LUT_tables_addr() { return _avx2_decode_lut_tables_base64; }
  static address compress_perm_table32() { return _compress_perm_table32; }
  static address compress_perm_table64() { return _compress_perm_table64; }
  static address expand_perm_table32() { return _expand_perm_table32; }
  static address expand_perm_table64() { return _expand_perm_table64; }
#endif
  static address pshuffle_byte_flip_mask_addr() { return _pshuffle_byte_flip_mask_addr; }
  static address pshuffle_byte_flip_mask_off32_addr() { return _pshuffle_byte_flip_mask_off32_addr; }
  static address pshuffle_byte_flip_mask_off64_addr() { return _pshuffle_byte_flip_mask_off64_addr; }
  static address arrays_hashcode_powers_of_31() { return (address)_arrays_hashcode_powers_of_31; }
  static void generate_CRC32C_table(bool is_pclmulqdq_supported);

#if _LP64
  // accessors for constants moved from stubGenerator_x86_64_aes.cpp
  static address key_shuffle_mask_addr() { return (address)_key_shuffle_mask; }
  static address counter_shuffle_mask_addr() { return (address)_counter_shuffle_mask; }
  static address counter_mask_linc0_addr() { return (address)_counter_mask_linc0; }
  static address counter_mask_linc1_addr() { return (address)_counter_mask_linc1; }
  static address counter_mask_linc1f_addr() { return (address)_counter_mask_linc1f; }
  static address counter_mask_linc2_addr() { return (address)_counter_mask_linc2; }
  static address counter_mask_linc2f_addr() { return (address)_counter_mask_linc2f; }
  static address counter_mask_linc4_addr() { return (address)_counter_mask_linc4; }
  static address counter_mask_linc8_addr() { return (address)_counter_mask_linc8; }
  static address counter_mask_linc16_addr() { return (address)_counter_mask_linc16; }
  static address counter_mask_linc32_addr() { return (address)_counter_mask_linc32; }
  static address counter_mask_ones_addr() { return (address)_counter_mask_ones; }
  static address ghash_polynomial_reduction_addr() { return (address)_ghash_polynomial_reduction; }
  static address ghash_polynomial_two_one_addr() { return (address) _ghash_polynomial_two_one; }

  // accessors for constants moved from stubGenerator_x86_64_ghash.cpp
  static address ghash_shuffle_mask_addr() { return (address)_ghash_shuffle_mask; }
  static address ghash_long_swap_mask_addr() { return (address)_ghash_long_swap_mask; }
  static address ghash_byte_swap_mask_addr() { return (address)_ghash_byte_swap_mask; }
  static address ghash_polynomial_addr() { return (address)_ghash_polynomial; }

  // accessors for constants moved from stubGenerator_x86_64_adler.cpp
  static address adler32_ascale_table_addr() { return (address)_adler32_ascale_table; }
  static address adler32_shuf0_table_addr() { return (address)_adler32_shuf0_table; }
  static address adler32_shuf1_table_addr() { return (address)_adler32_shuf1_table; }

  // accessors for constants moved from stubGenerator_x86_64_chacha.cpp
  static address chacha20_ctradd_avx_addr() { return (address) _cc20_counter_add_avx; }
  static address chacha20_ctradd_avx512_addr() { return (address) _cc20_counter_add_avx512; }
  static address chacha20_lrot_consts_addr() { return(address) _cc20_lrot_consts; }

  // accessors for constants moved from stubGenerator_x86_64_poly.cpp
  static address poly1305_pad_msg_addr() { return (address) _poly1305_pad_msg; }
  static address poly1305_mask42_addr() { return (address) _poly1305_mask42; }
  static address poly1305_mask44_addr() { return (address) _poly1305_mask44; }
#endif // _LP64
};

#endif // CPU_X86_STUBROUTINES_X86_HPP
