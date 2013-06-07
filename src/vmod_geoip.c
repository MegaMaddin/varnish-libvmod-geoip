/*-
 * Copyright (c) 2010-2011 Varnish Software AS
 * Copyright (c) 2013 Martin Probst
 * All rights reserved.
 *
 * Author: Poul-Henning Kamp <phk@FreeBSD.org>
 * Author: Martin Probst <github@megamaddin.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <GeoIP.h>
#include "vrt.h"
#include "bin/varnishd/cache.h"

#include "vcc_if.h"

#define VMOD_NAME "vmod_geoip"

typedef struct vmod_geoip {
    unsigned    magic;
#define VMGEOIP_MAGIC 0xafda74a9
    GeoIP       *geoip;
} vmod_geoip;

static const char *not_found = "NULL";

void __match_proto__()
vmod_load_geoip_db(struct sess *sp, struct vmod_priv *priv, const char *geoip_db)
{
    (void)sp;
    AN(priv);

    vmod_geoip *vmg = (vmod_geoip *)priv->priv;
    CHECK_OBJ_NOTNULL(vmg, VMGEOIP_MAGIC);

    if(!(access(geoip_db, R_OK))) {
        VSL(SLT_Debug, 0, "%s: loading GeoIP-DB %s", VMOD_NAME, geoip_db);
        vmg->geoip = GeoIP_open(geoip_db, GEOIP_MMAP_CACHE);
        if(vmg->geoip == NULL) 
            VSL(SLT_Debug, 0, "%s: failed loading %s: %s", VMOD_NAME, geoip_db, strerror(errno));
    } else {
        VSL(SLT_Debug, 0, "%s: failed to access %s: %s", VMOD_NAME, geoip_db, strerror(errno));
    }
}

const char * __match_proto__()
vmod_get_country_code(struct sess *sp, struct vmod_priv *priv, const char *ip)
{
    vmod_geoip *vmg = (vmod_geoip *)priv->priv;
    CHECK_OBJ_NOTNULL(sp, SESS_MAGIC);
    CHECK_OBJ_NOTNULL(vmg, VMGEOIP_MAGIC);
    const char *cc;

    if(vmg->geoip == NULL) {
        VSL(SLT_Debug, 0, "%s: GeoIP is not correctly initialized, maybe failed to load the database?", VMOD_NAME);
        cc = not_found;
        return(cc);
    }

    if(ip == NULL) {
        VSL(SLT_Debug, 0, "%s: IP string is NULL, passed the correct header?", VMOD_NAME);
        cc = not_found;
        return(cc);
    }

    cc = GeoIP_country_code_by_addr(vmg->geoip, ip);

    if(cc == NULL)
        cc = not_found;

    return(cc);
}

const char * __match_proto__()
vmod_get_country_name(struct sess *sp, struct vmod_priv *priv, const char *ip)
{
    vmod_geoip *vmg = (vmod_geoip *)priv->priv;
    CHECK_OBJ_NOTNULL(sp, SESS_MAGIC);
    CHECK_OBJ_NOTNULL(vmg, VMGEOIP_MAGIC);
    const char *cn;
 
    if(vmg->geoip == NULL) {
        VSL(SLT_Debug, 0, "%s: GeoIP is not correctly initialized, maybe failed to load the database?", VMOD_NAME);
        cn = not_found;
        return(cn);
    }

    if(ip == NULL) {
        VSL(SLT_Debug, 0, "%s: IP string is NULL, passed the correct header?", VMOD_NAME);
        cn = not_found;
        return(cn);
    }

    cn = GeoIP_country_name_by_addr(vmg->geoip, ip);

    if(cn == NULL)
        cn = not_found;

    return(cn);
}

void
geoip_cleanup(void *gi)
{
    vmod_geoip *vmg = gi;
    CHECK_OBJ_NOTNULL(vmg, VMGEOIP_MAGIC);

    GeoIP_delete(vmg->geoip);
    FREE_OBJ(vmg);
}

int
geoip_init(struct vmod_priv *priv, const struct VCL_conf *cfg)
{
    (void)cfg;

    vmod_geoip *vmg;
    ALLOC_OBJ(vmg, VMGEOIP_MAGIC);
    AN(vmg);

    priv->priv = vmg;
    priv->free = geoip_cleanup;
    return (0);
}
