/**
 * Copyright 2001-2004 KnowNow, Inc.  All rights reserved.
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
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the KnowNow, Inc., nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 * 
 *
 * $Id: lineartest.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "util.h"
#include "linear.h"

static unused char rcsid[] = "@(#) $Id: lineartest.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $";

int main()
{
    regression_t *r = new_regression();
    if (!r) 
    {
        fprintf(stderr, "Couldn't allocate regression struct\n");
        return 1;
    }
    
    regression_enter_data(r, 0, 0.1);
    regression_enter_data(r, 1, 1.1);
    assert(fabs(regression_slope(r) - 1.0) < 1e-6);
    assert(fabs(regression_intercept(r) - 0.1) < 1e-6);
    assert(fabs(regression_r(r) - 1.0) < 1e-6);

    regression_enter_data(r, 2, -5);
    assert(regression_slope(r) < 0);
    assert(regression_intercept(r) > 0);
    assert(regression_r(r) < 0.8);
    return 0;
}
