#include <stdint.h>
#include "klessydra_defs.h"

#define MHARTID_IDCORE_MASK 15

#ifndef THREAD_POOL_SIZE
	#define THREAD_POOL_SIZE 3
#endif

/*
If the user adds more scratchpad in the rtl, he must define more scratchpads belwo i.e. spmaddrE, spmaddrF etc..
If the user changes the starting address of the scratchpads in the rtl, he must also change the define values below
*/

#ifdef KLESS_SPM_A
#define spmaddrA KLESS_SPM_A
#endif 
#ifdef KLESS_SPM_B
#define spmaddrB KLESS_SPM_B
#endif
#ifdef KLESS_SPM_C
#define spmaddrC KLESS_SPM_C
#endif
#ifdef KLESS_SPM_D
#define spmaddrD KLESS_SPM_D
#endif

#ifndef KLESS_SPM_A
#define spmaddrA 0x10000000  // Default startung address of SPMA
#endif
#ifndef KLESS_SPM_B
#define spmaddrB 0x10001000  // Default startung address of SPMB
#endif
#ifndef KLESS_SPM_C
#define spmaddrC 0x10002000  // Default startung address of SPMC
#endif
#ifndef KLESS_SPM_D
#define spmaddrD 0x10003000  // Default startung address of SPMD
#endif

#ifndef __KLESSYDRACFUNCTIONS_H__
#define __KLESSYDRACFUNCTIONS_H__

int Klessydra_get_coreID();

__attribute__ ((always_inline)) inline void CSR_MVSIZE(int MYSIZE)
{
	__asm__(
		"csrw 0xBF0, %0;"
		://no output register
		:"r" (MYSIZE)
		:/*no clobbered register*/
	);	
}

__attribute__ ((always_inline)) inline void CSR_MVTYPE(int MVTYPE)
{
	__asm__(
		"csrw 0xBF8, %0;"
		://no output register
		:"r" (MVTYPE)
		:/*no clobbered register*/
	);	
}

__attribute__ ((always_inline)) inline int CSR_MPSCLFAC(int MPSCLFAC)
{
	__asm__(
        "csrw 0xBE0, %[p_scal];"
		://no output register
		:[p_scal] "r" (MPSCLFAC)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kmemld(void* rd, void* rs1, int rs2)
{
	__asm__(
		"kmemld %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return rs2;
}

__attribute__ ((always_inline)) inline int kbcastld(void* rd, void* rs1, int rs2)
{
	__asm__(
		"kbcastld %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return rs2;
}

__attribute__ ((always_inline)) inline int kmemstr(void* rd, void* rs1, int rs2)
{
	__asm__(
		"kmemstr %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:"memory"
	);
	
	return rs2;
}

__attribute__ ((always_inline)) inline int kaddv(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"kaddv %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kaddv_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"kaddv %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int ksvaddrf(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksvaddrf %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksvaddrf_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksvaddrf %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int ksvaddsc(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksvaddsc %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksvaddsc_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksvaddsc %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int ksubv(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksubv %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksubv_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksubv %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kvmul(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"kvmul %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kvmul_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"kvmul %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int ksvmulrf(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksvmulrf %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksvmulrf_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksvmulrf %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline  int ksvmulsc(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksvmulsc %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksvmulsc_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksvmulsc %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kvmuladd(void* rd, void* rs1, void* rs2, void* rs3)
{
	__asm__(
		"ksvmulsc %[rd], %[rs2], %[rs3];"
		"kaddv %[rd], %[rd], %[rs1];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2), [rs3] "r" (rs3)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kvmuladd_v2(void* rd, void* rs1, void* rs2, void* rs3, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksvmulsc %[rd], %[rs2], %[rs3];"
		"kaddv %[rd], %[rd], %[rs1];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2), [rs3] "r" (rs3)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kdotp(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"kdotp %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kdotp_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"kdotp %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kdotpps(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"kdotpps %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kdotpps_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"kdotpps %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kdotpps_v3(void* rd, void* rs1, void* rs2, void* p_scal)
{
	__asm__(
        "csrw 0xBE0, %[p_scal];"
		"kdotpps %[rd], %[rs1], %[rs2];"
		://no output register
		:[p_scal] "r" (p_scal), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kdotpps_v4(void* rd, void* rs1, void* rs2, void* p_scal,  int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
        "csrw 0xBE0, %[p_scal];"
		"kdotpps %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [p_scal] "r" (p_scal), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kdotpps_emul_v3(void* rd, void* rs1, void* rs2, void* p_scal)
{
	__asm__(
        "kvmul %[rd], %[rs1], %[rs2];"
        "ksrav %[rd], %[rd], %[p_scal];"
        "kvred %[rd], %[rd];"
		://no output register
		:[p_scal] "r" (p_scal), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kdotpps_emul_v4(void* rd, void* rs1, void* rs2, void* p_scal, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
        "kvmul %[rd], %[rs1], %[rs2];"
        "ksrav %[rd], %[rd], %[p_scal];"
        "kvred %[rd], %[rd];"
		://no output register
		:[p_scal] "r" (p_scal), [size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kvred(void* rd, void* rs1)
{
	__asm__(
		"kvred %[rd], %[rs1];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kvred_v2(void* rd, void* rs1, int size)
{
	__asm__(
		"csrw 0xBF0, %[size];"
		"kvred %[rd], %[rs1];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int ksrav(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksrav %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksrav_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksrav %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int ksrlv(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksrlv %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksrlv_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
		"csrw 0xBF0, %[size];"
		"ksrlv %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int krelu(void* rd, void* rs1)
{
	__asm__(
		"krelu %[rd], %[rs1];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int krelu_v2(void* rd, void* rs1, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"krelu %[rd], %[rs1];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kvslt(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"kvslt %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kvslt_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"kvslt %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int ksvslt(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksvslt %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksvslt_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksvslt %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kbcast(void* rd, void* rs1)
{
	__asm__(
		"kbcast %[rd], %[rs1];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kbcast_v2(void* rd, void* rs1, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"kbcast %[rd], %[rs1];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int kvcp(void* rd, void* rs1)
{
	__asm__(
		"kvcp %[rd], %[rs1];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kvcp_v2(void* rd, void* rs1, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"kvcp %[rd], %[rs1];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1)
		:/*no clobbered registers*/
	);
	
	return 1;
}
__attribute__ ((always_inline)) inline int kvdiv(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"kvdiv %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kvdiv_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"kvdiv %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int ksvdivrf(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksvdivrf %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline  int ksvdivsc(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksvdivsc %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}



__attribute__ ((always_inline)) inline int kvrem(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"kvrem %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int kvrem_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"kvrem %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int ksvremrf(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksvremrf %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksvremrf_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksvremrf %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline  int ksvremsc(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"ksvremsc %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int ksvremsc_v2(void* rd, void* rs1, void* rs2, int size)
{
	__asm__(
        "csrw 0xBF0, %[size];"
		"ksvremsc %[rd], %[rs1], %[rs2];"
		://no output register
		:[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return 1;
}

__attribute__ ((always_inline)) inline int hvbundle(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"hvbundle %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int hvclip(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"hvclip %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int hvbind(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"hvbind %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}
__attribute__ ((always_inline)) inline int hvperm(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"hvperm %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int hvsim(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"hvsim %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int hvsearch(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"hvsearch %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int hvenc(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"hvenc %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

__attribute__ ((always_inline)) inline int hvdec(void* rd, void* rs1, void* rs2)
{
	__asm__(
		"hvdec %[rd], %[rs1], %[rs2];"
		://no output register
		:[rd] "r" (rd), [rs1] "r" (rs1), [rs2] "r" (rs2)
		:/*no clobbered registers*/
	);
	
	return sizeof(rd);
}

void kaddv_complete(void *result, void* src1, void* src2, int size);
void kaddv_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksubv_complete(void *result, void* src1, void* src2, int size);
void ksubv_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void kvmul_complete(void *result, void* src1, void* src2, int size);
void kvmul_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void kvdiv_complete(void *result, void* src1, void* src2, int size);
void kvdiv_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void kvslt_complete(void *result, void* src1, void* src2, int size);
void kvslt_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void krelu_complete(void *result, void* src1, void* src2, int size);
void krelu_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksrlv_complete(void *result, void* src1, void* src2, int size);
void ksrlv_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksrav_complete(void *result, void* src1, void* src2, int size);
void ksrav_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksvaddsc_complete(void *result, void* src1, void* src2, int size);
void ksvaddsc_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksvaddrf_complete(void *result, void* src1, void* src2, int size);
void ksvaddrf_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksvmulsc_complete(void *result, void* src1, void* src2, int size);
void ksvmulsc_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksvmulrf_complete(void *result, void* src1, void* src2, int size);
void ksvmulrf_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksvdivsc_complete(void *result, void* src1, void* src2, int size);
void ksvdivsc_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksvdivrf_complete(void *result, void* src1, void* src2, int size);
void ksvdivrf_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksvremsc_complete(void *result, void* src1, void* src2, int size);
void ksvremsc_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksvremrf_complete(void *result, void* src1, void* src2, int size);
void ksvremrf_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

void ksvslt_complete(void *result, void* src1, void* src2, int size);
void ksvslt_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES);

// SW loop implementations of these functions are not made either because of their complexity or irrelevance
void kvred_complete(void *result, void* src1, void* src2, int size);

void kdotp_complete(void *result, void* src1, void* src2, int size);

void* kdotpps_complete(void *result, void* src1, void* src2, void* p_scal, int size);

#endif


#ifndef A_ORDER
#define A_ORDER 16 //Matrix size, don't do 2x2 case, for that there is another test
#endif
#ifndef B_ORDER
#define B_ORDER 3
#endif
#define Z_OVERHEAD B_ORDER-1 // total zeropadded overhead from both side of a row or a column
#define Z_ORDER (A_ORDER+Z_OVERHEAD)
#define Z_OFFSET Z_OVERHEAD>>1 // the data offset from the beginning of a line

#ifndef SIZE_OF_INT
#define SIZE_OF_INT 4
#endif

void matrix_check( int* mat1, int* mat2, int size );
void matrix_print(int* pt, int size, int mode, int start_row, int finish_row);

void conv2D(int fm_size, int krn_size, int (*matrix)[], int (*kernel)[], int (*out)[]);

// loop unrolled convolution function for kernels sizes from 3x3 to 11x11
void conv2D_3x3(int fm_size, int (*matrix)[], int (*kernel)[3], int (*out)[]);
void conv2D_5x5(int fm_size, int (*matrix)[], int (*kernel)[5], int (*out)[]);
void conv2D_7x7(int fm_size, int (*matrix)[], int (*kernel)[7], int (*out)[]);
void conv2D_9x9(int fm_size, int (*matrix)[], int (*kernel)[9], int (*out)[]);
void conv2D_11x11(int fm_size, int (*matrix)[], int (*kernel)[11], int (*out)[]);

void conv2D_accl(int fm_size, int krn_size, void* spm_fm, void* spm_krn, void* spm_res);