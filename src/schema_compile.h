/**
 * @file schema_compile.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Header for schema compilation.
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_SCHEMA_COMPILE_H_
#define LY_SCHEMA_COMPILE_H_

#include "log.h"
#include "schema_compile_node.h"
#include "set.h"
#include "tree_schema.h"

/**
 * @defgroup scflags Schema compile flags
 *
 * Flags are currently used only internally - the compilation process does not have a public interface and it is
 * integrated in the schema parsers. The current options set does not make sense for public used, but it can be a way
 * to modify behavior of the compilation process in future.
 *
 * @{
 */
#define LYS_COMPILE_RPC_INPUT  LYS_CONFIG_W       /**< Internal option when compiling schema tree of RPC/action input */
#define LYS_COMPILE_RPC_OUTPUT LYS_CONFIG_R       /**< Internal option when compiling schema tree of RPC/action output */
#define LYS_COMPILE_RPC_MASK   LYS_CONFIG_MASK    /**< mask for the internal RPC options */
#define LYS_COMPILE_NOTIFICATION 0x08             /**< Internal option when compiling schema tree of Notification */

#define LYS_COMPILE_GROUPING   0x10               /** Compiling (validation) of a non-instantiated grouping.
                                                      In this case not all the restrictions are checked since they can be valid only
                                                      in the real placement of the grouping. TODO - what specifically is not done */
/** @} scflags */

/**
 * @brief internal context for compilation
 */
struct lysc_ctx {
    struct ly_ctx *ctx;
    struct lys_module *cur_mod; /**< module currently being compiled, used as the current module for unprefixed nodes */
    struct lysp_module *pmod;   /**< parsed module being processed, used for searching imports to resolve prefixed nodes */
    struct ly_set groupings;    /**< stack for groupings circular check */
    struct ly_set xpath;        /**< when/must to check */
    struct ly_set leafrefs;     /**< to validate leafref's targets */
    struct ly_set dflts;        /**< set of incomplete default values */
    struct ly_set tpdf_chain;
    struct ly_set augs;         /**< set of compiled non-applied top-level augments */
    struct ly_set devs;         /**< set of compiled non-applied deviations */
    struct ly_set uses_augs;    /**< set of compiled non-applied uses augments */
    struct ly_set uses_rfns;    /**< set of compiled non-applied uses refines */
    uint32_t path_len;
    uint32_t options;           /**< various @ref scflags. */
#define LYSC_CTX_BUFSIZE 4078
    char path[LYSC_CTX_BUFSIZE];
};

/**
 * @brief Structure for remembering default values of leaves and leaf-lists. They are resolved at schema compilation
 * end when the whole schema tree is available.
 */
struct lysc_unres_dflt {
    union {
        struct lysc_node_leaf *leaf;
        struct lysc_node_leaflist *llist;
    };
    struct lysp_qname *dflt;
    struct lysp_qname *dflts;           /**< this is a sized array */
};

/**
 * @brief Duplicate string into dictionary
 * @param[in] CTX libyang context of the dictionary.
 * @param[in] ORIG String to duplicate.
 * @param[out] DUP Where to store the result.
 */
#define DUP_STRING(CTX, ORIG, DUP, RET) if (ORIG) {RET = lydict_insert(CTX, ORIG, 0, &DUP);}

#define DUP_STRING_GOTO(CTX, ORIG, DUP, RET, GOTO) if (ORIG) {LY_CHECK_GOTO(RET = lydict_insert(CTX, ORIG, 0, &DUP), GOTO);}

#define DUP_ARRAY(CTX, ORIG_ARRAY, NEW_ARRAY, DUP_FUNC) \
    if (ORIG_ARRAY) { \
        LY_ARRAY_COUNT_TYPE u; \
        LY_ARRAY_CREATE_RET(CTX, NEW_ARRAY, LY_ARRAY_COUNT(ORIG_ARRAY), LY_EMEM); \
        LY_ARRAY_FOR(ORIG_ARRAY, u) { \
            LY_ARRAY_INCREMENT(NEW_ARRAY); \
            LY_CHECK_RET(DUP_FUNC(CTX, &(NEW_ARRAY)[u], &(ORIG_ARRAY)[u])); \
        } \
    }

#define COMPILE_OP_ARRAY_GOTO(CTX, ARRAY_P, ARRAY_C, PARENT, ITER, FUNC, USES_STATUS, RET, GOTO) \
    if (ARRAY_P) { \
        LY_ARRAY_CREATE_GOTO((CTX)->ctx, ARRAY_C, LY_ARRAY_COUNT(ARRAY_P), RET, GOTO); \
        LY_ARRAY_COUNT_TYPE __array_offset = LY_ARRAY_COUNT(ARRAY_C); \
        for (ITER = 0; ITER < LY_ARRAY_COUNT(ARRAY_P); ++ITER) { \
            LY_ARRAY_INCREMENT(ARRAY_C); \
            RET = FUNC(CTX, &(ARRAY_P)[ITER], PARENT, &(ARRAY_C)[ITER + __array_offset], USES_STATUS); \
            if (RET == LY_EDENIED) { \
                LY_ARRAY_DECREMENT(ARRAY_C); \
            } else if (RET != LY_SUCCESS) { \
                goto GOTO; \
            } \
        } \
    }

#define COMPILE_ARRAY_GOTO(CTX, ARRAY_P, ARRAY_C, ITER, FUNC, RET, GOTO) \
    if (ARRAY_P) { \
        LY_ARRAY_CREATE_GOTO((CTX)->ctx, ARRAY_C, LY_ARRAY_COUNT(ARRAY_P), RET, GOTO); \
        LY_ARRAY_COUNT_TYPE __array_offset = LY_ARRAY_COUNT(ARRAY_C); \
        for (ITER = 0; ITER < LY_ARRAY_COUNT(ARRAY_P); ++ITER) { \
            LY_ARRAY_INCREMENT(ARRAY_C); \
            RET = FUNC(CTX, &(ARRAY_P)[ITER], &(ARRAY_C)[ITER + __array_offset]); \
            LY_CHECK_GOTO(RET != LY_SUCCESS, GOTO); \
        } \
    }

#define COMPILE_EXTS_GOTO(CTX, EXTS_P, EXT_C, PARENT, PARENT_TYPE, RET, GOTO) \
    if (EXTS_P) { \
        LY_ARRAY_CREATE_GOTO((CTX)->ctx, EXT_C, LY_ARRAY_COUNT(EXTS_P), RET, GOTO); \
        for (LY_ARRAY_COUNT_TYPE __exts_iter = 0, __array_offset = LY_ARRAY_COUNT(EXT_C); __exts_iter < LY_ARRAY_COUNT(EXTS_P); ++__exts_iter) { \
            LY_ARRAY_INCREMENT(EXT_C); \
            RET = lys_compile_ext(CTX, &(EXTS_P)[__exts_iter], &(EXT_C)[__exts_iter + __array_offset], PARENT, PARENT_TYPE, NULL); \
            LY_CHECK_GOTO(RET != LY_SUCCESS, GOTO); \
        } \
    }

/**
 * @brief Fill in the prepared compiled extension instance structure according to the parsed extension instance.
 *
 * @param[in] ctx Compilation context.
 * @param[in] ext_p Parsed extension instance.
 * @param[in,out] ext Prepared compiled extension instance.
 * @param[in] parent Extension instance parent.
 * @param[in] parent_type Extension instance parent type.
 * @param[in] ext_mod Optional module with the extension instance extension definition, set only for internal annotations.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_ext(struct lysc_ctx *ctx, struct lysp_ext_instance *ext_p, struct lysc_ext_instance *ext, void *parent,
        LYEXT_PARENT parent_type, const struct lys_module *ext_mod);

/**
 * @brief Compile information from the if-feature statement
 * @param[in] ctx Compile context.
 * @param[in] qname The if-feature argument to process. It is pointer-to-qname just to unify the compile functions.
 * @param[in,out] iff Prepared (empty) compiled if-feature structure to fill.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_iffeature(struct lysc_ctx *ctx, struct lysp_qname *qname, struct lysc_iffeature *iff);

/**
 * @brief Compile information from the identity statement
 *
 * The backlinks to the identities derived from this one are supposed to be filled later via ::lys_compile_identity_bases().
 *
 * @param[in] ctx_sc Compile context - alternative to the combination of @p ctx and @p parsed_mod.
 * @param[in] ctx libyang context.
 * @param[in] parsed_mod Module with the identities.
 * @param[in] identities_p Array of the parsed identity definitions to precompile.
 * @param[in,out] identities Pointer to the storage of the (pre)compiled identities array where the new identities are
 * supposed to be added. The storage is supposed to be initiated to NULL when the first parsed identities are going
 * to be processed.
 * @return LY_ERR value.
 */
LY_ERR lys_identity_precompile(struct lysc_ctx *ctx_sc, struct ly_ctx *ctx, struct lysp_module *parsed_mod,
        struct lysp_ident *identities_p, struct lysc_ident **identities);

/**
 * @brief Find and process the referenced base identities from another identity or identityref
 *
 * For bases in identity set backlinks to them from the base identities. For identityref, store
 * the array of pointers to the base identities. So one of the ident or bases parameter must be set
 * to distinguish these two use cases.
 *
 * @param[in] ctx Compile context, not only for logging but also to get the current module to resolve prefixes.
 * @param[in] base_pmod Module where to resolve @p bases_p prefixes.
 * @param[in] bases_p Array of names (including prefix if necessary) of base identities.
 * @param[in] ident Referencing identity to work with, NULL for identityref.
 * @param[in] bases Array of bases of identityref to fill in.
 * @return LY_ERR value.
 */
LY_ERR lys_compile_identity_bases(struct lysc_ctx *ctx, const struct lysp_module *base_pmod, const char **bases_p,
        struct lysc_ident *ident, struct lysc_ident ***bases);

/**
 * @brief Create pre-compiled features array.
 *
 * Features are compiled in two steps to allow forward references between them via their if-feature statements.
 * In case of not implemented schemas, the precompiled list of features is stored in lys_module structure and
 * the compilation is not finished (if-feature and extensions are missing) and all the features are permanently
 * disabled without a chance to change it. The list is used as target for any if-feature statement in any
 * implemented module to get valid data to evaluate its result. The compilation is finished via
 * ::lys_feature_precompile_finish() in implemented modules. In case a not implemented module becomes implemented,
 * the precompiled list is reused to finish the compilation to preserve pointers already used in various compiled
 * if-feature structures.
 *
 * @param[in] ctx_sc Compile context - alternative to the combination of @p ctx and @p parsed_mod.
 * @param[in] ctx libyang context.
 * @param[in] parsed_mod Module with the features.
 * @param[in] features_p Array of the parsed features definitions to precompile.
 * @param[in,out] features Pointer to the storage of the (pre)compiled features array where the new features are
 * supposed to be added. The storage is supposed to be initiated to NULL when the first parsed features are going
 * to be processed.
 * @return LY_ERR value.
 */
LY_ERR lys_feature_precompile(struct lysc_ctx *ctx_sc, struct ly_ctx *ctx, struct lysp_module *parsed_mod,
        struct lysp_feature *features_p, struct lysc_feature **features);

/**
 * @brief Revert compiled list of features back to the precompiled state.
 *
 * Function is needed in case the compilation failed and the schema is expected to revert back to the non-compiled status.
 *
 * @param[in] ctx Compilation context.
 * @param[in] mod The module structure with the features to decompile.
 */
void lys_feature_precompile_revert(struct lysc_ctx *ctx, struct lys_module *mod);

/**
 * @brief Check statement's status for invalid combination.
 *
 * The modX parameters are used just to determine if both flags are in the same module,
 * so any of the schema module structure can be used, but both modules must be provided
 * in the same type.
 *
 * @param[in] ctx Compile context for logging.
 * @param[in] flags1 Flags of the referencing node.
 * @param[in] mod1 Module of the referencing node,
 * @param[in] name1 Schema node name of the referencing node.
 * @param[in] flags2 Flags of the referenced node.
 * @param[in] mod2 Module of the referenced node,
 * @param[in] name2 Schema node name of the referenced node.
 * @return LY_ERR value
 */
LY_ERR lysc_check_status(struct lysc_ctx *ctx, uint16_t flags1, void *mod1, const char *name1, uint16_t flags2,
        void *mod2, const char *name2);

/**
 * @brief Check parsed expression for any prefixes of unimplemented modules.
 *
 * @param[in] ctx libyang context.
 * @param[in] expr Parsed expression.
 * @param[in] format Prefix format.
 * @param[in] prefix_data Format-specific data (see ::ly_resolve_prefix()).
 * @param[in] implement Whether all the non-implemented modules should are implemented or the first
 * non-implemented module, if any, returned in @p mod_p.
 * @param[out] mod_p Module that is not implemented.
 * @return LY_SUCCESS on success.
 * @return LY_ERR on error.
 */
LY_ERR lys_compile_expr_implement(const struct ly_ctx *ctx, const struct lyxp_expr *expr, LY_PREFIX_FORMAT format,
        void *prefix_data, ly_bool implement, const struct lys_module **mod_p);

/**
 * @brief Compile printable schema into a validated schema linking all the references.
 *
 * @param[in] mod Pointer to the schema structure holding pointers to both schema structure types. The ::lys_module#parsed
 * member is used as input and ::lys_module#compiled is used to hold the result of the compilation.
 * @param[in] options Various options to modify compiler behavior, see [compile flags](@ref scflags).
 * @return LY_ERR value - LY_SUCCESS or LY_EVALID.
 */
LY_ERR lys_compile(struct lys_module *mod, uint32_t options);

#endif /* LY_SCHEMA_COMPILE_H_ */