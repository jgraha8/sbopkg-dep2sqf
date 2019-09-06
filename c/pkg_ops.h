#ifndef __PKG_OPS_H__
#define __PKG_OPS_H__

#include <stdbool.h>

#include "pkg_graph.h"


bool pkg_db_exists();
bool pkg_reviewed_exists();


int pkg_write_db(pkg_nodes_t *pkgs);
int pkg_write_reviewed(pkg_nodes_t *pkgs);

int pkg_create_default_deps(pkg_nodes_t *pkgs);

int pkg_load_db(pkg_nodes_t *pkgs);
int pkg_load_reviewed(pkg_nodes_t *pkgs);
int pkg_load_sbo(pkg_nodes_t *pkgs);

int pkg_load_dep(struct pkg_graph *pkg_graph, const char *pkg_name, struct pkg_options options);
int pkg_load_all_deps(struct pkg_graph *pkg_graph, struct pkg_options options);

int pkg_review(const struct pkg *pkg);
int pkg_review_prompt(const struct pkg *pkg);
int pkg_show_info(const struct pkg *pkg);
int pkg_compar_sets(const pkg_nodes_t *new_pkgs, pkg_nodes_t *old_pkgs);

#endif