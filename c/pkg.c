#include <assert.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <libbds/bds_queue.h>
#include <libbds/bds_stack.h>

#include "file_mmap.h"
#include "pkg.h"
#include "pkg_util.h"
#include "sbo.h"
#include "slack_pkg.h"
#include "user_config.h"

#ifndef MAX_LINE
#define MAX_LINE 2048
#endif

struct pkg_options pkg_options_default()
{
        struct pkg_options options;
        memset(&options, 0, sizeof(options));

        options.recursive = true;
        options.optional  = true;

        return options;
}

struct pkg pkg_create(const char *name)
{
        struct pkg pkg;

        memset(&pkg, 0, sizeof(pkg));

        pkg.name = bds_string_dup(name);

        return pkg;
}

/* struct pkg pkg_create_reference(const struct pkg *pkg_src) */
/* { */
/* 	struct pkg pkg_dst; */

/* 	pkg_dst = *pkg_src; */
/* 	pkg_dst.owner = false; */
/* } */

void pkg_destroy(struct pkg *pkg)
{
        if (pkg->name)
                free(pkg->name);
        if (pkg->version)
                free(pkg->version);
        if (pkg->sbo_dir)
                free(pkg->sbo_dir);

        if (pkg->dep.required) {
                bds_vector_free(&pkg->dep.required);
        }
        if (pkg->dep.parents) {
                bds_vector_free(&pkg->dep.parents);
        }
        if (pkg->dep.buildopts)
                bds_vector_free(&pkg->dep.buildopts);

        memset(pkg, 0, sizeof(*pkg));
}

void pkg_copy_nodep(struct pkg *pkg_dst, const struct pkg *pkg_src)
{
        pkg_destroy(pkg_dst);

        pkg_dst->name = bds_string_dup(pkg_src->name);

        if (pkg_src->version)
                pkg_dst->version = bds_string_dup(pkg_src->version);
        if (pkg_src->sbo_dir)
                pkg_dst->sbo_dir = bds_string_dup(pkg_src->sbo_dir);
        pkg_dst->info_crc        = pkg_src->info_crc;

        return;
}

void pkg_init_version(struct pkg *pkg, const char *version)
{
        assert(pkg->version == NULL);
        pkg->version = bds_string_dup(version);
}
void pkg_set_version(struct pkg *pkg, const char *version)
{
        assert(pkg->version != NULL);
        free(pkg->version);
        pkg->version = bds_string_dup(version);
}

void pkg_init_sbo_dir(struct pkg *pkg, const char *sbo_dir)
{
        assert(pkg->sbo_dir == NULL);
        pkg->sbo_dir = bds_string_dup(sbo_dir);
}

int pkg_set_info_crc(struct pkg *pkg)
{
        if (pkg->sbo_dir == NULL)
                return 1;

        char *readme         = sbo_load_readme(pkg->sbo_dir, pkg->name);
        const char *requires = sbo_read_requires(pkg->sbo_dir, pkg->name);

        pkg->info_crc = crc32_z(0L, Z_NULL, 0);
        pkg->info_crc = crc32_z(pkg->info_crc, (unsigned char *)readme, strlen(readme));
        pkg->info_crc = crc32_z(pkg->info_crc, (unsigned char *)requires, strlen(requires));

        free(readme);

        return 0;
}

static void __pkg_insert_dep(pkg_nodes_t **pkg_list, struct pkg_node *dep_node)
{
        if (*pkg_list == NULL) {
                *pkg_list = pkg_nodes_alloc_reference();
        }

        if (pkg_nodes_bsearch(*pkg_list, dep_node->pkg.name))
                return;

        pkg_nodes_insert_sort(*pkg_list, dep_node);
}

void pkg_insert_required(struct pkg *pkg, struct pkg_node *req_node)
{
        __pkg_insert_dep(&pkg->dep.required, req_node);
        //        bds_vector_append(pkg->dep.required, &req);
}

void pkg_insert_parent(struct pkg *pkg, struct pkg_node *parent_node)
{
        __pkg_insert_dep(&pkg->dep.parents, parent_node);
        //        bds_vector_append(pkg->dep.required, &req);
}

struct pkg *__pkg_bsearch_dep(pkg_nodes_t *pkg_list, const char *dep_name)
{
        if (pkg_list == NULL)
                return NULL;

        struct pkg_node *node = pkg_nodes_bsearch(pkg_list, dep_name);

        if (node == NULL)
                return NULL;

        return &node->pkg;
}

struct pkg *pkg_bsearch_required(struct pkg *pkg, const char *req_name)
{
        return __pkg_bsearch_dep(pkg->dep.required, req_name);
}

struct pkg *pkg_bsearch_parent(struct pkg *pkg, const char *parent_name)
{
        return __pkg_bsearch_dep(pkg->dep.parents, parent_name);
}

void pkg_append_buildopts(struct pkg *pkg, char *bopt)
{
        if (pkg->dep.buildopts == NULL) {
                pkg->dep.buildopts = bds_vector_alloc(1, sizeof(char *), (void (*)(void *))free_string_ptr);
        }
        bds_vector_append(pkg->dep.buildopts, &bopt);
}

size_t pkg_buildopts_size(const struct pkg *pkg)
{
        if (pkg->dep.buildopts == NULL)
                return 0;

        return bds_vector_size(pkg->dep.buildopts);
}

const char *pkg_buildopts_get_const(const struct pkg *pkg, size_t i)
{
        if (pkg->dep.buildopts == NULL)
                return 0;

        const char **bp = (const char **)bds_vector_get(pkg->dep.buildopts, i);
        if (bp == NULL)
                return NULL;

        return *bp;
}
