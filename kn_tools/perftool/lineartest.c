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
 * $Id: lineartest.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $
 **/

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "util.h"
#include "linear.h"

static unused char rcsid[] = "@(#) $Id: lineartest.c,v 1.1 2003/03/21 05:23:56 ifindkarma Exp $";

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
