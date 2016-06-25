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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gstterminator.h"

GST_DEBUG_CATEGORY_STATIC (gst_terminator_debug_category);
#define GST_CAT_DEFAULT gst_terminator_debug_category

#define TIMEOUT_DEFAULT_VALUE 0

/* prototypes */


static void gst_terminator_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_terminator_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec);

static void gst_terminator_dispose (GObject * object);
static void gst_terminator_finalize (GObject * object);

static GstCaps *gst_terminator_transform_caps (GstBaseTransform * trans, GstPadDirection direction, GstCaps * caps, GstCaps * filter);
static GstCaps *gst_terminator_fixate_caps (GstBaseTransform * trans, GstPadDirection direction, GstCaps * caps, GstCaps * othercaps);
static gboolean gst_terminator_accept_caps (GstBaseTransform * trans, GstPadDirection direction, GstCaps * caps);
static gboolean gst_terminator_set_caps (GstBaseTransform * trans, GstCaps * incaps, GstCaps * outcaps);
static gboolean gst_terminator_query (GstBaseTransform * trans, GstPadDirection direction, GstQuery * query);
static gboolean gst_terminator_decide_allocation (GstBaseTransform * trans, GstQuery * query);
static gboolean gst_terminator_filter_meta (GstBaseTransform * trans, GstQuery * query, GType api, const GstStructure * params);
static gboolean gst_terminator_propose_allocation (GstBaseTransform * trans, GstQuery * decide_query, GstQuery * query);
static gboolean gst_terminator_transform_size (GstBaseTransform * trans, GstPadDirection direction, GstCaps * caps, gsize size, GstCaps * othercaps, gsize * othersize);
static gboolean gst_terminator_get_unit_size (GstBaseTransform * trans, GstCaps * caps, gsize * size);
static gboolean gst_terminator_start (GstBaseTransform * trans);
static gboolean gst_terminator_stop (GstBaseTransform * trans);
static gboolean gst_terminator_sink_event (GstBaseTransform * trans, GstEvent * event);
static gboolean gst_terminator_src_event (GstBaseTransform * trans, GstEvent * event);
static GstFlowReturn gst_terminator_prepare_output_buffer (GstBaseTransform * trans, GstBuffer * input, GstBuffer ** outbuf);
static gboolean gst_terminator_copy_metadata (GstBaseTransform * trans, GstBuffer * input, GstBuffer * outbuf);
static gboolean gst_terminator_transform_meta (GstBaseTransform * trans, GstBuffer * outbuf, GstMeta * meta, GstBuffer * inbuf);
static void gst_terminator_before_transform (GstBaseTransform * trans, GstBuffer * buffer);
static GstFlowReturn gst_terminator_transform (GstBaseTransform * trans, GstBuffer * inbuf, GstBuffer * outbuf);
static GstFlowReturn gst_terminator_transform_ip (GstBaseTransform * trans, GstBuffer * buf);

static gboolean gst_terminator_timeout(GstClock *clock, GstClockTime time, GstClockID id, gpointer user_data);

enum
{
  PROP_0,
  PROP_TIMEOUT
};

/* pad templates */

static GstStaticPadTemplate gst_terminator_src_template =
GST_STATIC_PAD_TEMPLATE (GST_BASE_TRANSFORM_SRC_NAME,
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY
    );

static GstStaticPadTemplate gst_terminator_sink_template =
GST_STATIC_PAD_TEMPLATE (GST_BASE_TRANSFORM_SINK_NAME,
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstTerminator, gst_terminator, GST_TYPE_BASE_TRANSFORM,
  GST_DEBUG_CATEGORY_INIT (gst_terminator_debug_category, "terminator", 0,
  "debug category for terminator element"));

static void
gst_terminator_class_init (GstTerminatorClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_static_pad_template_get (&gst_terminator_src_template));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_static_pad_template_get (&gst_terminator_sink_template));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "Terminator",
      "Generic",
      "Set options for terminating application gracefully",
      "<@example.com>");

  gobject_class->set_property = gst_terminator_set_property;
  gobject_class->get_property = gst_terminator_get_property;

  g_object_class_install_property (gobject_class, PROP_TIMEOUT,
      g_param_spec_uint("timeout", "timeout", "application timeout in seconds",
      0, G_MAXUINT32, TIMEOUT_DEFAULT_VALUE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));


//   gobject_class->dispose = gst_terminator_dispose;
//   gobject_class->finalize = gst_terminator_finalize;
//   base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_terminator_transform_caps);
//   base_transform_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_terminator_fixate_caps);
//   base_transform_class->accept_caps = GST_DEBUG_FUNCPTR (gst_terminator_accept_caps);
//   base_transform_class->set_caps = GST_DEBUG_FUNCPTR (gst_terminator_set_caps);
//   base_transform_class->query = GST_DEBUG_FUNCPTR (gst_terminator_query);
//   base_transform_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_terminator_decide_allocation);
//   base_transform_class->filter_meta = GST_DEBUG_FUNCPTR (gst_terminator_filter_meta);
//   base_transform_class->propose_allocation = GST_DEBUG_FUNCPTR (gst_terminator_propose_allocation);
//   base_transform_class->transform_size = GST_DEBUG_FUNCPTR (gst_terminator_transform_size);
//   base_transform_class->get_unit_size = GST_DEBUG_FUNCPTR (gst_terminator_get_unit_size);
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_terminator_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_terminator_stop);
//   base_transform_class->sink_event = GST_DEBUG_FUNCPTR (gst_terminator_sink_event);
//   base_transform_class->src_event = GST_DEBUG_FUNCPTR (gst_terminator_src_event);
//   base_transform_class->prepare_output_buffer = GST_DEBUG_FUNCPTR (gst_terminator_prepare_output_buffer);
//   base_transform_class->copy_metadata = GST_DEBUG_FUNCPTR (gst_terminator_copy_metadata);
//   base_transform_class->transform_meta = GST_DEBUG_FUNCPTR (gst_terminator_transform_meta);
//   base_transform_class->before_transform = GST_DEBUG_FUNCPTR (gst_terminator_before_transform);
//   base_transform_class->transform = GST_DEBUG_FUNCPTR (gst_terminator_transform);
//   base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_terminator_transform_ip);

}

static void
gst_terminator_init (GstTerminator *terminator)
{
    terminator->timeout = TIMEOUT_DEFAULT_VALUE;
}

void
gst_terminator_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstTerminator *terminator = GST_TERMINATOR (object);

  GST_DEBUG_OBJECT (terminator, "set_property");

  switch (property_id) {
    case PROP_TIMEOUT:
      terminator->timeout = g_value_get_uint(value);
      GST_DEBUG("timeout=%d\n", terminator->timeout);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_terminator_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstTerminator *terminator = GST_TERMINATOR (object);

  GST_DEBUG_OBJECT (terminator, "get_property");

  switch (property_id) {
    case PROP_TIMEOUT:
      g_value_set_uint(value, terminator->timeout);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_terminator_dispose (GObject * object)
{
  GstTerminator *terminator = GST_TERMINATOR (object);

  GST_DEBUG_OBJECT (terminator, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_terminator_parent_class)->dispose (object);
}

void
gst_terminator_finalize (GObject * object)
{
  GstTerminator *terminator = GST_TERMINATOR (object);

  GST_DEBUG_OBJECT (terminator, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_terminator_parent_class)->finalize (object);
}

static GstCaps *
gst_terminator_transform_caps (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps, GstCaps * filter)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);
  GstCaps *othercaps;

  GST_DEBUG_OBJECT (terminator, "transform_caps");

  othercaps = gst_caps_copy (caps);

  /* Copy other caps and modify as appropriate */
  /* This works for the simplest cases, where the transform modifies one
   * or more fields in the caps structure.  It does not work correctly
   * if passthrough caps are preferred. */
  if (direction == GST_PAD_SRC) {
    /* transform caps going upstream */
  } else {
    /* transform caps going downstream */
  }

  if (filter) {
    GstCaps *intersect;

    intersect = gst_caps_intersect (othercaps, filter);
    gst_caps_unref (othercaps);

    return intersect;
  } else {
    return othercaps;
  }
}

static GstCaps *
gst_terminator_fixate_caps (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps, GstCaps * othercaps)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "fixate_caps");

  return NULL;
}

static gboolean
gst_terminator_accept_caps (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "accept_caps");

  return TRUE;
}

static gboolean
gst_terminator_set_caps (GstBaseTransform * trans, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "set_caps");

  return TRUE;
}

static gboolean
gst_terminator_query (GstBaseTransform * trans, GstPadDirection direction,
    GstQuery * query)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "query");

  return TRUE;
}

/* decide allocation query for output buffers */
static gboolean
gst_terminator_decide_allocation (GstBaseTransform * trans, GstQuery * query)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "decide_allocation");

  return TRUE;
}

static gboolean
gst_terminator_filter_meta (GstBaseTransform * trans, GstQuery * query, GType api,
    const GstStructure * params)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "filter_meta");

  return TRUE;
}

/* propose allocation query parameters for input buffers */
static gboolean
gst_terminator_propose_allocation (GstBaseTransform * trans,
    GstQuery * decide_query, GstQuery * query)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "propose_allocation");

  return TRUE;
}

/* transform size */
static gboolean
gst_terminator_transform_size (GstBaseTransform * trans, GstPadDirection direction,
    GstCaps * caps, gsize size, GstCaps * othercaps, gsize * othersize)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "transform_size");

  return TRUE;
}

static gboolean
gst_terminator_get_unit_size (GstBaseTransform * trans, GstCaps * caps,
    gsize * size)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "get_unit_size");

  return TRUE;
}

static gboolean
gst_terminator_timeout(GstClock *clock, GstClockTime time, GstClockID id, gpointer user_data)
{
  GST_DEBUG("TIMEOUT: clockid=%p time=%llu", id, time);
  gst_element_send_event(GST_ELEMENT(user_data), gst_event_new_eos());
  return TRUE;
}

/* states */
static gboolean
gst_terminator_start (GstBaseTransform * trans)
{
  gboolean ret = TRUE;
  GstTerminator *terminator = GST_TERMINATOR (trans);
  int clock_ret;

  GST_DEBUG_OBJECT (terminator, "start");

  if (terminator->timeout != 0)
  {
	 GstClock *clock = gst_system_clock_obtain();
	 GstClockTime endtime = gst_clock_get_time(clock) + (terminator->timeout * GST_SECOND);
	 GstClockID clkid = gst_clock_new_single_shot_id(clock, endtime);

     GST_DEBUG("clockid=%p endtime=%llu", clkid, endtime);

	 clock_ret = gst_clock_id_wait_async(clkid, gst_terminator_timeout, trans, NULL);
     if (GST_CLOCK_OK != clock_ret)
     {
       GST_DEBUG("timeout failed %d", clock_ret);
       ret = FALSE;
     }
  }

  return ret;
}

static gboolean
gst_terminator_stop (GstBaseTransform * trans)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "stop");

  return TRUE;
}

/* sink and src pad event handlers */
static gboolean
gst_terminator_sink_event (GstBaseTransform * trans, GstEvent * event)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "sink_event");

  return GST_BASE_TRANSFORM_CLASS (gst_terminator_parent_class)->sink_event (
      trans, event);
}

static gboolean
gst_terminator_src_event (GstBaseTransform * trans, GstEvent * event)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "src_event");

  return GST_BASE_TRANSFORM_CLASS (gst_terminator_parent_class)->src_event (
      trans, event);
}

static GstFlowReturn
gst_terminator_prepare_output_buffer (GstBaseTransform * trans, GstBuffer * input,
    GstBuffer ** outbuf)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "prepare_output_buffer");

  return GST_FLOW_OK;
}

/* metadata */
static gboolean
gst_terminator_copy_metadata (GstBaseTransform * trans, GstBuffer * input,
    GstBuffer * outbuf)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "copy_metadata");

  return TRUE;
}

static gboolean
gst_terminator_transform_meta (GstBaseTransform * trans, GstBuffer * outbuf,
    GstMeta * meta, GstBuffer * inbuf)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "transform_meta");

  return TRUE;
}

static void
gst_terminator_before_transform (GstBaseTransform * trans, GstBuffer * buffer)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "before_transform");

}

/* transform */
static GstFlowReturn
gst_terminator_transform (GstBaseTransform * trans, GstBuffer * inbuf,
    GstBuffer * outbuf)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "transform");

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_terminator_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
  GstTerminator *terminator = GST_TERMINATOR (trans);

  GST_DEBUG_OBJECT (terminator, "transform_ip");

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "terminator", GST_RANK_NONE,
      GST_TYPE_TERMINATOR);
}

/* these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "utility"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "utility"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    terminator,
    "Set options for terminating application gracefully",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

