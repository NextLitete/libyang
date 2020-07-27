/**
 * @file printer_data.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Data printers for libyang
 *
 * Copyright (c) 2015-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_PRINTER_DATA_H_
#define LY_PRINTER_DATA_H_

#include <stdio.h>
#include <unistd.h>

#include "log.h"
#include "tree_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ly_out;

/**
 * @defgroup dataprinterflags Data printer flags
 * @ingroup datatree
 *
 * Validity flags for data nodes.
 *
 * @{
 */
#define LYD_PRINT_WITHSIBLINGS  0x01  /**< Flag for printing also the (following) sibling nodes of the data node. */
#define LYD_PRINT_FORMAT        0x02  /**< Flag for formatted output. */
#define LYD_PRINT_KEEPEMPTYCONT 0x04  /**< Preserve empty non-presence containers */
#define LYD_PRINT_WD_MASK       0xF0  /**< Mask for with-defaults modes */
#define LYD_PRINT_WD_EXPLICIT   0x00  /**< Explicit mode - print only data explicitly being present in the data tree.
                                           Note that this is the default value when no WD option is specified. */
#define LYD_PRINT_WD_TRIM       0x10  /**< Do not print the nodes with the value equal to their default value */
#define LYD_PRINT_WD_ALL        0x20  /**< Include implicit default nodes */
#define LYD_PRINT_WD_ALL_TAG    0x40  /**< Same as #LYDP_WD_ALL but also adds attribute 'default' with value 'true' to
                                           all nodes that has its default value. The 'default' attribute has namespace:
                                           urn:ietf:params:xml:ns:netconf:default:1.0 and thus the attributes are
                                           printed only when the ietf-netconf-with-defaults module is present in libyang
                                           context (but in that case this namespace is always printed). */
#define LYD_PRINT_WD_IMPL_TAG   0x80  /**< Same as LYDP_WD_ALL_TAG but the attributes are added only to the nodes that
                                           are not explicitly present in the original data tree despite their
                                           value is equal to their default value.  There is the same limitation regarding
                                           the presence of ietf-netconf-with-defaults module in libyang context. */
/**
 * @}
 */

/**
 * @brief Print the whole data tree of the root, including all the siblings.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] root The root element of the tree to print, can be any sibling.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags) except ::LYD_PRINT_WITHSIBLINGS.
 * @return LY_ERR value.
 */
LY_ERR lyd_print_all(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print the selected data subtree.
 *
 * @param[in] out Printer handler for a specific output. Use ly_out_*() functions to create and free the handler.
 * @param[in] root The root element of the subtree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags) except ::LYD_PRINT_WITHSIBLINGS.
 * @return LY_ERR value.
 */
LY_ERR lyd_print_tree(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[out] strp Pointer to store the resulting dump.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[in] fd File descriptor where to print the data.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[in] f File stream where to print the data.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[in] path File path where to print the data.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lyd_print_path(const char *path, const struct lyd_node *root, LYD_FORMAT format, int options);

/**
 * @brief Print data tree in the specified format.
 *
 * @param[in] writeclb Callback function to write the data (see write(1)).
 * @param[in] arg Optional caller-specific argument to be passed to the \p writeclb callback.
 * @param[in] root The root element of the (sub)tree to print.
 * @param[in] format Output format.
 * @param[in] options [Data printer flags](@ref dataprinterflags).
 * @return LY_ERR value.
 */
LY_ERR lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg,
                     const struct lyd_node *root, LYD_FORMAT format, int options);

#ifdef __cplusplus
}
#endif

#endif /* LY_PRINTER_DATA_H_ */