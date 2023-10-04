/*
 * This file is part of the EMSI Tools Package developed at the
 * Brain Research Laboratory, Middle East Technical University
 * Department of Electrical and Electronics Engineering.
 *
 * Copyright (C) 2008 Zeynep Akalin Acar
 * Copyright (C) 2008 Can Erkin Acar
 * Copyright (C) 2008 Nevzat G. Gencer
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
//---------------------------------------------------------------------------
#include "rect3.h"
//---------------------------------------------------------------------------

void Rect3::init(void)
{
    register double t;
    
    if (m_p1.getX() > m_p2.getX()){
        t=m_p1.getX();
        m_p1.setX(m_p2.getX());
        m_p2.setX(t);
    }

    if (m_p1.getY() > m_p2.getY()){
        double t=m_p1.getY();
        m_p1.setY(m_p2.getY());
        m_p2.setY(t);
    }

    if (m_p1.getZ() > m_p2.getZ()){
        double t=m_p1.getZ();
        m_p1.setZ(m_p2.getZ());
        m_p2.setZ(t);
    }
}
