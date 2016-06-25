/* GStreamer
 * Copyright (C) 2016 <@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_TERMINATOR_H_
#define _GST_TERMINATOR_H_

#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_TERMINATOR   (gst_terminator_get_type())
#define GST_TERMINATOR(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TERMINATOR,GstTerminator))
#define GST_TERMINATOR_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TERMINATOR,GstTerminatorClass))
#define GST_IS_TERMINATOR(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TERMINATOR))
#define GST_IS_TERMINATOR_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TERMINATOR))

typedef struct _GstTerminator GstTerminator;
typedef struct _GstTerminatorClass GstTerminatorClass;

struct _GstTerminator
{
  GstBaseTransform base_terminator;

  guint32 timeout;
};

struct _GstTerminatorClass
{
  GstBaseTransformClass base_terminator_class;
};

GType gst_terminator_get_type (void);

G_END_DECLS

#endif
