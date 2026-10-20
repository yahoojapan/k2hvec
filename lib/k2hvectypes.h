/*
 * K2HVEC
 *
 * Copyright 2026 LY Corporation.
 *
 * K2HVEC is a high-performance vector database for fast vector search,
 * built on top of the K2HASH key-value store.
 *
 * For the full copyright and license information, please view
 * the license file that was distributed with this source code.
 *
 * AUTHOR:   Takeshi Nakatani
 * CREATE:   Tue, Jul 21 2026
 * REVISION:
 *
 */

#ifndef	K2HVECTYPES_H
#define	K2HVECTYPES_H

DECL_EXTERN_C_START

//---------------------------------------------------------
// Typedefs
//---------------------------------------------------------
//
// Opaque handle representing a K2hVecDb object
//
// This handle is defined as an opaque pointer to a K2hVecDb instance,
// allowing it to be safely passed across the C API boundary without
// exposing the underlying C++ class structure.
//
typedef struct k2hvec_handle*	k2hvec_handle_t;

DECL_EXTERN_C_END

#endif	// K2HVECTYPES_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
