/**
 * Copyright (c) 2001-2003 KnowNow, Inc.  All rights reserved.
 *
 * @KNOWNOW_LICENSE_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * 
 * 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
 * be used to endorse or promote any product without prior written
 * permission from KnowNow, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 *
 * $Id: linear.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

/* 
 * linear regression.
 */
#include <math.h>
#include <stdlib.h>
#include "util.h"
#include "linear.h"

static unused char rcsid[] = "@(#) $Id: linear.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $";

struct regression {
    double xTotal;
    double yTotal;
    double xSquaredTotal;
    double ySquaredTotal;
    double xyTotal;
    double xMin, xMax, yMin, yMax;
    long n;
};

regression_t *new_regression()
{
    regression_t *rv = (regression_t*)malloc(sizeof(*rv));
    rv->xTotal = rv->xSquaredTotal = 0.0;
    rv->yTotal = rv->ySquaredTotal = rv->xyTotal = 0.0;
    rv->yMin = rv->yMax = rv->xMax = rv->xMin = 0.0/0.0;
    rv->n = 0;
    return rv;
}

void free_regression(regression_t *r)
{
    free(r);
}

void regression_enter_data(regression_t *r, double x, double y)
{
    r->xTotal += x;
    r->yTotal += y;
    r->xSquaredTotal += x*x;
    r->ySquaredTotal += y*y;
    r->xyTotal += x*y;
    r->n++;
    if (isnan(r->xMin) || x < r->xMin) r->xMin = x;
    if (isnan(r->xMax) || x > r->xMax) r->xMax = x;
    if (isnan(r->yMin) || y < r->yMin) r->yMin = y;
    if (isnan(r->yMax) || y > r->yMax) r->yMax = y;
}

double regression_slope(regression_t *r)
{
    return ((r->n * r->xyTotal - r->xTotal * r->yTotal) 
            /
            (r->n * r->xSquaredTotal - r->xTotal * r->xTotal));
}

double regression_intercept(regression_t *r)
{
    return (r->yTotal - regression_slope(r) * r->xTotal) / r->n;
}

double regression_r(regression_t *r)
{
    return ((r->n * r->xyTotal - r->xTotal * r->yTotal)
            /
            sqrt((r->n * r->xSquaredTotal - r->xTotal * r->xTotal) *
                 (r->n * r->ySquaredTotal - r->yTotal * r->yTotal)));
}

double regression_ymin(regression_t *r)
{
    return r->yMin;
}

double regression_ymax(regression_t *r)
{
    return r->yMax;
}

double regression_ymean(regression_t *r)
{
    return r->yTotal / r->n;
}

double regression_xmin(regression_t *r)
{
    return r->xMin;
}

double regression_xmax(regression_t *r)
{
    return r->xMax;
}

double regression_xmean(regression_t *r)
{
    return r->xTotal / r->n;
}

double regression_predicted_y_at_xmin(regression_t *r)
{
    return regression_intercept(r) + regression_slope(r) * regression_xmin(r);
}
