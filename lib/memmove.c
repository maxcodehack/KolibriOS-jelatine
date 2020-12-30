/***************************************************************************
 *   Copyright © 2005-2011 by Gabriele Svelto                              *
 *   gabriele.svelto@gmail.com                                             *
 *                                                                         *
 *   This file is part of Jelatine.                                        *
 *                                                                         *
 *   Jelatine is free software: you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Jelatine is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with Jelatine.  If not, see <http://www.gnu.org/licenses/>.     *
 ***************************************************************************/

#if HAVE_CONFIG_H
#   include <config.h>
#endif

#if ! HAVE_MEMMOVE

void *memmove(void *dest, const void *src, size_t n)
{
    char *tsrc;
    char *tdest;
    size_t i;

    tsrc = (char *) src;
    tdest = (char *) dest;
    
    if (tdest > tsrc)
    {
        for (i = 0; i < n; i++)
            tdest[n - i - 1] = tsrc[n - i - 1]
    }
    else
    {
        for (i = 0; i < n; i++)
            tdest[i] = tsrc[i];
    }
    
    return dest;
}

#endif
