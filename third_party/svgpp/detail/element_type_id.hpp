// Copyright Oleg Maximenko 2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://github.com/svgpp/svgpp for library home page.

#pragma once

namespace svgpp { namespace detail
{

  enum element_type_id
  {
    element_type_id_a,
    element_type_id_altGlyph,
    element_type_id_altGlyphDef,
    element_type_id_altGlyphItem,
    element_type_id_animate,
    element_type_id_animateColor,
    element_type_id_animateMotion,
    element_type_id_animateTransform,
    element_type_id_circle,
    element_type_id_clipPath,
    element_type_id_color_profile,
    element_type_id_cursor,
    element_type_id_defs,
    element_type_id_desc,
    element_type_id_ellipse,
    element_type_id_feBlend,
    element_type_id_feColorMatrix,
    element_type_id_feComponentTransfer,
    element_type_id_feComposite,
    element_type_id_feConvolveMatrix,
    element_type_id_feDiffuseLighting,
    element_type_id_feDisplacementMap,
    element_type_id_feDistantLight,
    element_type_id_feFlood,
    element_type_id_feFuncA,
    element_type_id_feFuncB,
    element_type_id_feFuncG,
    element_type_id_feFuncR,
    element_type_id_feGaussianBlur,
    element_type_id_feImage,
    element_type_id_feMerge,
    element_type_id_feMergeNode,
    element_type_id_feMorphology,
    element_type_id_feOffset,
    element_type_id_fePointLight,
    element_type_id_feSpecularLighting,
    element_type_id_feSpotLight,
    element_type_id_feTile,
    element_type_id_feTurbulence,
    element_type_id_filter,
    element_type_id_font,
    element_type_id_font_face,
    element_type_id_font_face_format,
    element_type_id_font_face_name,
    element_type_id_font_face_src,
    element_type_id_font_face_uri,
    element_type_id_foreignObject,
    element_type_id_g,
    element_type_id_glyph,
    element_type_id_glyphRef,
    element_type_id_hkern,
    element_type_id_image,
    element_type_id_line,
    element_type_id_linearGradient,
    element_type_id_marker,
    element_type_id_mask,
    element_type_id_metadata,
    element_type_id_missing_glyph,
    element_type_id_mpath,
    element_type_id_path,
    element_type_id_pattern,
    element_type_id_polygon,
    element_type_id_polyline,
    element_type_id_radialGradient,
    element_type_id_rect,
    element_type_id_script,
    element_type_id_set,
    element_type_id_stop,
    element_type_id_style,
    element_type_id_svg,
    element_type_id_switch_,
    element_type_id_symbol,
    element_type_id_text,
    element_type_id_textPath,
    element_type_id_title,
    element_type_id_tref,
    element_type_id_tspan,
    element_type_id_use_,
    element_type_id_view,
    element_type_id_vkern,
    element_type_count,
    unknown_element_type_id = element_type_count
  };

}}