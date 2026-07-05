function Get-XgeSvgCompareCases {
	param(
		[switch]$IncludeExperimental
	)

	$cases = @(
		[ordered]@{ name = "shapes"; svg = "examples\xge_svg\assets\shapes.svg" },
		[ordered]@{ name = "compat"; svg = "examples\xge_svg\assets\compat.svg" },
		[ordered]@{ name = "mask_text_image"; svg = "examples\xge_svg\assets\mask_text_image.svg" }
	)
	if ($IncludeExperimental) {
		$cases += [ordered]@{ name = "clip_vector"; svg = "examples\xge_svg\assets\clip_vector.svg"; experimental = $true }
		$cases += [ordered]@{ name = "clip_rule"; svg = "examples\xge_svg\assets\clip_rule.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_vector"; svg = "examples\xge_svg\assets\mask_vector.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_group_style"; svg = "examples\xge_svg\assets\mask_group_style.svg"; experimental = $true }
		$cases += [ordered]@{ name = "clip_mask_use"; svg = "examples\xge_svg\assets\clip_mask_use.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_region"; svg = "examples\xge_svg\assets\mask_region.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_stroke"; svg = "examples\xge_svg\assets\mask_stroke.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_text_image_vector"; svg = "examples\xge_svg\assets\mask_text_image_vector.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_offset"; svg = "examples\xge_svg\assets\filter_offset.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_drop_shadow"; svg = "examples\xge_svg\assets\filter_drop_shadow.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_region"; svg = "examples\xge_svg\assets\filter_region.svg"; experimental = $true }
		$cases += [ordered]@{ name = "raster_image"; svg = "examples\xge_svg\assets\raster_image.svg"; experimental = $true }
		$cases += [ordered]@{ name = "external_image"; svg = "examples\xge_svg\assets\external_image.svg"; experimental = $true }
		$cases += [ordered]@{ name = "external_use"; svg = "examples\xge_svg\assets\external_use.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_cross_href"; svg = "examples\xge_svg\assets\gradient_cross_href.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_units"; svg = "examples\xge_svg\assets\gradient_units.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_order"; svg = "examples\xge_svg\assets\gradient_stop_order.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_offset_invalid"; svg = "examples\xge_svg\assets\gradient_stop_offset_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "opacity_invalid"; svg = "examples\xge_svg\assets\opacity_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_mixed_content"; svg = "examples\xge_svg\assets\marker_mixed_content.svg"; experimental = $true }
		$cases += [ordered]@{ name = "paint_order_markers"; svg = "examples\xge_svg\assets\paint_order_markers.svg"; experimental = $true }
		$cases += [ordered]@{ name = "paint_order_invalid"; svg = "examples\xge_svg\assets\paint_order_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "paint_invalid"; svg = "examples\xge_svg\assets\paint_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "use_markers"; svg = "examples\xge_svg\assets\use_markers.svg"; experimental = $true }
		$cases += [ordered]@{ name = "use_inherit"; svg = "examples\xge_svg\assets\use_inherit.svg"; experimental = $true }
		$cases += [ordered]@{ name = "group_use"; svg = "examples\xge_svg\assets\group_use.svg"; experimental = $true }
		$cases += [ordered]@{ name = "svg_use"; svg = "examples\xge_svg\assets\svg_use.svg"; experimental = $true }
		$cases += [ordered]@{ name = "preserve_aspect_invalid"; svg = "examples\xge_svg\assets\preserve_aspect_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dash_join"; svg = "examples\xge_svg\assets\stroke_dash_join.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dasharray_invalid"; svg = "examples\xge_svg\assets\stroke_dasharray_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dash_negative"; svg = "examples\xge_svg\assets\stroke_dash_negative.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dashoffset_percent"; svg = "examples\xge_svg\assets\stroke_dashoffset_percent.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dashoffset_invalid"; svg = "examples\xge_svg\assets\stroke_dashoffset_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_linejoin"; svg = "examples\xge_svg\assets\stroke_linejoin.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_line_enum_invalid"; svg = "examples\xge_svg\assets\stroke_line_enum_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_miterlimit"; svg = "examples\xge_svg\assets\stroke_miterlimit.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_miterlimit_invalid"; svg = "examples\xge_svg\assets\stroke_miterlimit_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_miterlimit_negative"; svg = "examples\xge_svg\assets\stroke_miterlimit_negative.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_width_invalid"; svg = "examples\xge_svg\assets\stroke_width_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_roundrect"; svg = "examples\xge_svg\assets\stroke_roundrect.svg"; experimental = $true }
		$cases += [ordered]@{ name = "pattern_units"; svg = "examples\xge_svg\assets\pattern_units.svg"; experimental = $true }
		$cases += [ordered]@{ name = "pattern_stroke"; svg = "examples\xge_svg\assets\pattern_stroke.svg"; experimental = $true }
	}
	return $cases
}
