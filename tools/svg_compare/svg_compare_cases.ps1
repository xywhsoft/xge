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
		$cases += [ordered]@{ name = "clip_use_rule"; svg = "examples\xge_svg\assets\clip_use_rule.svg"; experimental = $true }
		$cases += [ordered]@{ name = "clip_mixed_union"; svg = "examples\xge_svg\assets\clip_mixed_union.svg"; experimental = $true }
		$cases += [ordered]@{ name = "clip_multi_mixed_union"; svg = "examples\xge_svg\assets\clip_multi_mixed_union.svg"; experimental = $true }
		$cases += [ordered]@{ name = "clip_nested_clip_path"; svg = "examples\xge_svg\assets\clip_nested_clip_path.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_vector"; svg = "examples\xge_svg\assets\mask_vector.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_group_style"; svg = "examples\xge_svg\assets\mask_group_style.svg"; experimental = $true }
		$cases += [ordered]@{ name = "clip_mask_use"; svg = "examples\xge_svg\assets\clip_mask_use.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_region"; svg = "examples\xge_svg\assets\mask_region.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_stroke"; svg = "examples\xge_svg\assets\mask_stroke.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mask_text_image_vector"; svg = "examples\xge_svg\assets\mask_text_image_vector.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_offset"; svg = "examples\xge_svg\assets\filter_offset.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_drop_shadow"; svg = "examples\xge_svg\assets\filter_drop_shadow.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_drop_shadow_primitive"; svg = "examples\xge_svg\assets\filter_drop_shadow_primitive.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_region"; svg = "examples\xge_svg\assets\filter_region.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_gaussian_blur"; svg = "examples\xge_svg\assets\filter_gaussian_blur.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_offset_blur"; svg = "examples\xge_svg\assets\filter_offset_blur.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_offset_chain"; svg = "examples\xge_svg\assets\filter_offset_chain.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_offset_source_alpha"; svg = "examples\xge_svg\assets\filter_offset_source_alpha.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_blur_source_alpha"; svg = "examples\xge_svg\assets\filter_blur_source_alpha.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_color_matrix"; svg = "examples\xge_svg\assets\filter_color_matrix.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_component_transfer"; svg = "examples\xge_svg\assets\filter_component_transfer.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_flood"; svg = "examples\xge_svg\assets\filter_flood.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_current_color"; svg = "examples\xge_svg\assets\filter_current_color.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_shadow_pipeline"; svg = "examples\xge_svg\assets\filter_shadow_pipeline.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_blend"; svg = "examples\xge_svg\assets\filter_blend.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_color_graph"; svg = "examples\xge_svg\assets\filter_color_graph.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_color_graph_multi"; svg = "examples\xge_svg\assets\filter_color_graph_multi.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_independent_color_graph"; svg = "examples\xge_svg\assets\filter_independent_color_graph.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_identity_graph"; svg = "examples\xge_svg\assets\filter_identity_graph.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_identity"; svg = "examples\xge_svg\assets\filter_morphology_identity.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_independent"; svg = "examples\xge_svg\assets\filter_morphology_independent.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_region"; svg = "examples\xge_svg\assets\filter_morphology_region.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_source_alpha"; svg = "examples\xge_svg\assets\filter_morphology_source_alpha.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_source_erode"; svg = "examples\xge_svg\assets\filter_morphology_source_erode.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_ellipse_erode"; svg = "examples\xge_svg\assets\filter_morphology_ellipse_erode.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_tile_independent"; svg = "examples\xge_svg\assets\filter_tile_independent.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_image"; svg = "examples\xge_svg\assets\filter_image.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_image_data_fragment"; svg = "examples\xge_svg\assets\filter_image_data_fragment.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_image_local_ref"; svg = "examples\xge_svg\assets\filter_image_local_ref.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_image_url_fragment"; svg = "examples\xge_svg\assets\filter_image_url_fragment.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_primitive_region"; svg = "examples\xge_svg\assets\filter_primitive_region.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_offset_independent_region"; svg = "examples\xge_svg\assets\filter_offset_independent_region.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_composite_region"; svg = "examples\xge_svg\assets\filter_composite_region.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_composite"; svg = "examples\xge_svg\assets\filter_composite.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_merge"; svg = "examples\xge_svg\assets\filter_merge.svg"; experimental = $true }
		$cases += [ordered]@{ name = "filter_units_invalid"; svg = "examples\xge_svg\assets\filter_units_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "raster_image"; svg = "examples\xge_svg\assets\raster_image.svg"; experimental = $true }
		$cases += [ordered]@{ name = "image_data_uri_policy"; svg = "examples\xge_svg\assets\image_data_uri_policy.svg"; experimental = $true }
		$cases += [ordered]@{ name = "image_svg_data_charset"; svg = "examples\xge_svg\assets\image_svg_data_charset.svg"; experimental = $true }
		$cases += [ordered]@{ name = "image_svg_data_fragment"; svg = "examples\xge_svg\assets\image_svg_data_fragment.svg"; experimental = $true }
		$cases += [ordered]@{ name = "image_jpg_data_uri"; svg = "examples\xge_svg\assets\image_jpg_data_uri.svg"; experimental = $true }
		$cases += [ordered]@{ name = "external_image"; svg = "examples\xge_svg\assets\external_image.svg"; experimental = $true }
		$cases += [ordered]@{ name = "external_use"; svg = "examples\xge_svg\assets\external_use.svg"; experimental = $true }
		$cases += [ordered]@{ name = "use_data_uri_fragment"; svg = "examples\xge_svg\assets\use_data_uri_fragment.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_cross_href"; svg = "examples\xge_svg\assets\gradient_cross_href.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_units"; svg = "examples\xge_svg\assets\gradient_units.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_order"; svg = "examples\xge_svg\assets\gradient_stop_order.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_offset_invalid"; svg = "examples\xge_svg\assets\gradient_stop_offset_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_style_order"; svg = "examples\xge_svg\assets\gradient_stop_style_order.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_case"; svg = "examples\xge_svg\assets\gradient_stop_case.svg"; experimental = $true }
		$cases += [ordered]@{ name = "gradient_radial_fr"; svg = "examples\xge_svg\assets\gradient_radial_fr.svg"; experimental = $true }
		$cases += [ordered]@{ name = "enum_inherit_invalid"; svg = "examples\xge_svg\assets\enum_inherit_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "style_invalid_keywords"; svg = "examples\xge_svg\assets\style_invalid_keywords.svg"; experimental = $true }
		$cases += [ordered]@{ name = "style_duplicate_invalid"; svg = "examples\xge_svg\assets\style_duplicate_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "text_style_duplicate_invalid"; svg = "examples\xge_svg\assets\text_style_duplicate_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "color_function_grammar"; svg = "examples\xge_svg\assets\color_function_grammar.svg"; experimental = $true }
		$cases += [ordered]@{ name = "hsl_grammar"; svg = "examples\xge_svg\assets\hsl_grammar.svg"; experimental = $true }
		$cases += [ordered]@{ name = "css_property_case"; svg = "examples\xge_svg\assets\css_property_case.svg"; experimental = $true }
		$cases += [ordered]@{ name = "css_keyword_case"; svg = "examples\xge_svg\assets\css_keyword_case.svg"; experimental = $true }
		$cases += [ordered]@{ name = "css_attribute_case_flag"; svg = "examples\xge_svg\assets\css_attribute_case_flag.svg"; experimental = $true }
		$cases += [ordered]@{ name = "css_identifier_escape"; svg = "examples\xge_svg\assets\css_identifier_escape.svg"; experimental = $true }
		$cases += [ordered]@{ name = "css_attribute_escape"; svg = "examples\xge_svg\assets\css_attribute_escape.svg"; experimental = $true }
		$cases += [ordered]@{ name = "css_url_escape"; svg = "examples\xge_svg\assets\css_url_escape.svg"; experimental = $true }
		$cases += [ordered]@{ name = "css_nth_spacing"; svg = "examples\xge_svg\assets\css_nth_spacing.svg"; experimental = $true }
		$cases += [ordered]@{ name = "css_pseudo_case"; svg = "examples\xge_svg\assets\css_pseudo_case.svg"; experimental = $true }
		$cases += [ordered]@{ name = "css_media_rule"; svg = "examples\xge_svg\assets\css_media_rule.svg"; experimental = $true }
		$cases += [ordered]@{ name = "opacity_invalid"; svg = "examples\xge_svg\assets\opacity_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "opacity_duplicate_invalid"; svg = "examples\xge_svg\assets\opacity_duplicate_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "mix_blend_mode"; svg = "examples\xge_svg\assets\mix_blend_mode.svg"; experimental = $true }
		$cases += [ordered]@{ name = "font_size_invalid"; svg = "examples\xge_svg\assets\font_size_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "font_size_duplicate_invalid"; svg = "examples\xge_svg\assets\font_size_duplicate_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "text_baseline_shift"; svg = "examples\xge_svg\assets\text_baseline_shift.svg"; experimental = $true }
		$cases += [ordered]@{ name = "length_suffix"; svg = "examples\xge_svg\assets\length_suffix.svg"; experimental = $true }
		$cases += [ordered]@{ name = "xml_markup_boundaries"; svg = "examples\xge_svg\assets\xml_markup_boundaries.svg"; experimental = $true }
		$cases += [ordered]@{ name = "xml_entities"; svg = "examples\xge_svg\assets\xml_entities.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_mixed_content"; svg = "examples\xge_svg\assets\marker_mixed_content.svg"; experimental = $true }
		$cases += [ordered]@{ name = "paint_order_markers"; svg = "examples\xge_svg\assets\paint_order_markers.svg"; experimental = $true }
		$cases += [ordered]@{ name = "paint_order_invalid"; svg = "examples\xge_svg\assets\paint_order_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "paint_invalid"; svg = "examples\xge_svg\assets\paint_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "paint_invalid_gradient"; svg = "examples\xge_svg\assets\paint_invalid_gradient.svg"; experimental = $true }
		$cases += [ordered]@{ name = "url_reference_invalid"; svg = "examples\xge_svg\assets\url_reference_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "url_function_case"; svg = "examples\xge_svg\assets\url_function_case.svg"; experimental = $true }
		$cases += [ordered]@{ name = "transform_angle_units"; svg = "examples\xge_svg\assets\transform_angle_units.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_orient_angle_units"; svg = "examples\xge_svg\assets\marker_orient_angle_units.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_overflow_clip"; svg = "examples\xge_svg\assets\marker_overflow_clip.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_ref_keywords"; svg = "examples\xge_svg\assets\marker_ref_keywords.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_bounds_hit"; svg = "examples\xge_svg\assets\marker_bounds_hit.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_quadratic_path"; svg = "examples\xge_svg\assets\marker_quadratic_path.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_arc_mid_vertex"; svg = "examples\xge_svg\assets\marker_arc_mid_vertex.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_closed_path"; svg = "examples\xge_svg\assets\marker_closed_path.svg"; experimental = $true }
		$cases += [ordered]@{ name = "marker_invalid_reference"; svg = "examples\xge_svg\assets\marker_invalid_reference.svg"; experimental = $true }
		$cases += [ordered]@{ name = "use_markers"; svg = "examples\xge_svg\assets\use_markers.svg"; experimental = $true }
		$cases += [ordered]@{ name = "use_inherit"; svg = "examples\xge_svg\assets\use_inherit.svg"; experimental = $true }
		$cases += [ordered]@{ name = "group_use"; svg = "examples\xge_svg\assets\group_use.svg"; experimental = $true }
		$cases += [ordered]@{ name = "svg_use"; svg = "examples\xge_svg\assets\svg_use.svg"; experimental = $true }
		$cases += [ordered]@{ name = "symbol_overflow"; svg = "examples\xge_svg\assets\symbol_overflow.svg"; experimental = $true }
		$cases += [ordered]@{ name = "nested_svg_viewport_clip"; svg = "examples\xge_svg\assets\nested_svg_viewport_clip.svg"; experimental = $true }
		$cases += [ordered]@{ name = "clip_mask_nested_svg"; svg = "examples\xge_svg\assets\clip_mask_nested_svg.svg"; experimental = $true }
		$cases += [ordered]@{ name = "switch_conditional"; svg = "examples\xge_svg\assets\switch_conditional.svg"; experimental = $true }
		$cases += [ordered]@{ name = "switch_style_scope"; svg = "examples\xge_svg\assets\switch_style_scope.svg"; experimental = $true }
		$cases += [ordered]@{ name = "switch_selector_scope"; svg = "examples\xge_svg\assets\switch_selector_scope.svg"; experimental = $true }
		$cases += [ordered]@{ name = "switch_container_inherit"; svg = "examples\xge_svg\assets\switch_container_inherit.svg"; experimental = $true }
		$cases += [ordered]@{ name = "conditional_processing_attrs"; svg = "examples\xge_svg\assets\conditional_processing_attrs.svg"; experimental = $true }
		$cases += [ordered]@{ name = "anchor_container"; svg = "examples\xge_svg\assets\anchor_container.svg"; experimental = $true }
		$cases += [ordered]@{ name = "metadata_skip"; svg = "examples\xge_svg\assets\metadata_skip.svg"; experimental = $true }
		$cases += [ordered]@{ name = "xlink_href"; svg = "examples\xge_svg\assets\xlink_href.svg"; experimental = $true }
		$cases += [ordered]@{ name = "rect_invalid_radii"; svg = "examples\xge_svg\assets\rect_invalid_radii.svg"; experimental = $true }
		$cases += [ordered]@{ name = "points_invalid"; svg = "examples\xge_svg\assets\points_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "points_invalid_clip_mask"; svg = "examples\xge_svg\assets\points_invalid_clip_mask.svg"; experimental = $true }
		$cases += [ordered]@{ name = "path_invalid"; svg = "examples\xge_svg\assets\path_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "path_incomplete_commands"; svg = "examples\xge_svg\assets\path_incomplete_commands.svg"; experimental = $true }
		$cases += [ordered]@{ name = "path_number_grammar"; svg = "examples\xge_svg\assets\path_number_grammar.svg"; experimental = $true }
		$cases += [ordered]@{ name = "path_quadratic"; svg = "examples\xge_svg\assets\path_quadratic.svg"; experimental = $true }
		$cases += [ordered]@{ name = "number_grammar_attrs"; svg = "examples\xge_svg\assets\number_grammar_attrs.svg"; experimental = $true }
		$cases += [ordered]@{ name = "length_number_grammar"; svg = "examples\xge_svg\assets\length_number_grammar.svg"; experimental = $true }
		$cases += [ordered]@{ name = "circle_ellipse_attrs"; svg = "examples\xge_svg\assets\circle_ellipse_attrs.svg"; experimental = $true }
		$cases += [ordered]@{ name = "path_length"; svg = "examples\xge_svg\assets\path_length.svg"; experimental = $true }
		$cases += [ordered]@{ name = "path_length_dash_hit"; svg = "examples\xge_svg\assets\path_length_dash_hit.svg"; experimental = $true }
		$cases += [ordered]@{ name = "line_path_length_dash_hit"; svg = "examples\xge_svg\assets\line_path_length_dash_hit.svg"; experimental = $true }
		$cases += [ordered]@{ name = "preserve_aspect_invalid"; svg = "examples\xge_svg\assets\preserve_aspect_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dasharray_odd"; svg = "examples\xge_svg\assets\stroke_dasharray_odd.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dash_join"; svg = "examples\xge_svg\assets\stroke_dash_join.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dasharray_invalid"; svg = "examples\xge_svg\assets\stroke_dasharray_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dash_negative"; svg = "examples\xge_svg\assets\stroke_dash_negative.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dashoffset_percent"; svg = "examples\xge_svg\assets\stroke_dashoffset_percent.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dashoffset_invalid"; svg = "examples\xge_svg\assets\stroke_dashoffset_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_duplicate_invalid"; svg = "examples\xge_svg\assets\stroke_duplicate_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_dash_inherit"; svg = "examples\xge_svg\assets\stroke_dash_inherit.svg"; experimental = $true }
		$cases += [ordered]@{ name = "style_inherit_core"; svg = "examples\xge_svg\assets\style_inherit_core.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_linejoin"; svg = "examples\xge_svg\assets\stroke_linejoin.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_zero_length_caps"; svg = "examples\xge_svg\assets\stroke_zero_length_caps.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_line_enum_invalid"; svg = "examples\xge_svg\assets\stroke_line_enum_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_miterlimit"; svg = "examples\xge_svg\assets\stroke_miterlimit.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_miterlimit_invalid"; svg = "examples\xge_svg\assets\stroke_miterlimit_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_miterlimit_negative"; svg = "examples\xge_svg\assets\stroke_miterlimit_negative.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_width_invalid"; svg = "examples\xge_svg\assets\stroke_width_invalid.svg"; experimental = $true }
		$cases += [ordered]@{ name = "stroke_roundrect"; svg = "examples\xge_svg\assets\stroke_roundrect.svg"; experimental = $true }
		$cases += [ordered]@{ name = "pattern_units"; svg = "examples\xge_svg\assets\pattern_units.svg"; experimental = $true }
		$cases += [ordered]@{ name = "pattern_stroke"; svg = "examples\xge_svg\assets\pattern_stroke.svg"; experimental = $true }
		$cases += [ordered]@{ name = "pattern_overflow"; svg = "examples\xge_svg\assets\pattern_overflow.svg"; experimental = $true }
	}
	return $cases
}

function Assert-XgeSvgCompareCasesSelected {
	param(
		[object[]]$Cases
	)

	if (($Cases -eq $null) -or ($Cases.Count -eq 0)) {
		throw "No SVG compare cases selected."
	}
}

function Add-XgeSvgCompareCaseTag {
	param(
		[string[]]$Tags,
		[string]$Tag
	)

	if (($Tag -eq $null) -or ($Tag -eq "")) {
		return @($Tags)
	}
	$normalized = $Tag.ToLowerInvariant()
	if (@($Tags | Where-Object { $_ -eq $normalized }).Count -eq 0) {
		return @($Tags + $normalized)
	}
	return @($Tags)
}

function Get-XgeSvgCompareCaseTags {
	param(
		[object]$Case
	)

	$tags = @()
	if ($Case -eq $null) {
		return $tags
	}
	$name = [string]$Case.name
	$kind = if ($Case.Contains("experimental") -and $Case.experimental) { "experimental" } else { "stable" }
	$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag $kind
	if ($Case.Contains("tags")) {
		foreach ($tag in @($Case.tags)) {
			$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag ([string]$tag)
		}
	}
	if ($name -match "^[^_]+") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag $matches[0]
	}
	if (($name -eq "shapes") -or ($name -match "circle|ellipse|rect|points|roundrect")) {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "shape"
	}
	if ($name -match "(^|_)path") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "path"
	}
	if ($name -match "(^|_)filter|^fe") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "filter"
	}
	if ($name -match "(^|_)clip") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "clip"
	}
	if ($name -match "(^|_)mask") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "mask"
	}
	if ($name -match "(^|_)marker|markers") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "marker"
	}
	if ($name -match "(^|_)gradient") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "gradient"
	}
	if ($name -match "(^|_)pattern") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "pattern"
	}
	if ($name -match "(^|_)stroke") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "stroke"
	}
	if ($name -match "(^|_)text|font|baseline") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "text"
	}
	if ($name -match "(^|_)image|raster|jpg|png|webp") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "image"
	}
	if ($name -match "(^|_)use|xlink|symbol") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "use"
	}
	if ($name -match "(^|_)css|style|color|hsl") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "css"
	}
	if ($name -match "switch|conditional|required") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "conditional"
	}
	if ($name -match "length|number|opacity|paint|url|transform|xml|metadata|anchor|enum|preserve") {
		$tags = Add-XgeSvgCompareCaseTag -Tags $tags -Tag "grammar"
	}
	return @($tags)
}

function Test-XgeSvgCompareCaseTag {
	param(
		[object]$Case,
		[string]$Pattern
	)

	foreach ($tag in Get-XgeSvgCompareCaseTags -Case $Case) {
		if (($tag -eq $Pattern) -or ($tag -like $Pattern)) {
			return $true
		}
	}
	return $false
}

function Expand-XgeSvgComparePatterns {
	param(
		[string[]]$Patterns
	)

	$expanded = @()
	if ($Patterns -eq $null) {
		return $expanded
	}
	foreach ($pattern in $Patterns) {
		if ($pattern -eq $null) {
			continue
		}
		foreach ($part in ([string]$pattern).Split(",")) {
			$value = $part.Trim()
			if ($value -ne "") {
				$expanded += $value
			}
		}
	}
	return $expanded
}

function Select-XgeSvgCompareCases {
	param(
		[object[]]$Cases,
		[string[]]$CaseName = @(),
		[string[]]$CaseTag = @()
	)

	$pool = @($Cases)
	$CaseName = Expand-XgeSvgComparePatterns -Patterns $CaseName
	$CaseTag = Expand-XgeSvgComparePatterns -Patterns $CaseTag
	Assert-XgeSvgCompareCasesSelected -Cases $pool

	if (($CaseTag -ne $null) -and ($CaseTag.Count -gt 0)) {
		$tagSelected = @()
		$effectiveTagPatternCount = 0
		foreach ($pattern in $CaseTag) {
			if (($pattern -eq $null) -or ($pattern -eq "")) {
				continue
			}
			$effectiveTagPatternCount++
			$matches = @($pool | Where-Object { Test-XgeSvgCompareCaseTag -Case $_ -Pattern $pattern.ToLowerInvariant() })
			if ($matches.Count -eq 0) {
				throw "SVG compare case tag not found: $pattern"
			}
			foreach ($case in $matches) {
				if (@($tagSelected | Where-Object { $_.name -eq $case.name }).Count -eq 0) {
					$tagSelected += $case
				}
			}
		}
		if ($effectiveTagPatternCount -eq 0) {
			throw "SVG compare case tag filter is empty."
		}
		Assert-XgeSvgCompareCasesSelected -Cases $tagSelected
		$pool = @($tagSelected)
	}

	if (($CaseName -eq $null) -or ($CaseName.Count -eq 0)) {
		Assert-XgeSvgCompareCasesSelected -Cases $pool
		return $pool
	}

	$selected = @()
	$effectivePatternCount = 0
	foreach ($pattern in $CaseName) {
		if (($pattern -eq $null) -or ($pattern -eq "")) {
			continue
		}
		$effectivePatternCount++
		$matches = @($pool | Where-Object { ($_.name -eq $pattern) -or ($_.name -like $pattern) })
		if ($matches.Count -eq 0) {
			throw "SVG compare case not found: $pattern"
		}
		foreach ($case in $matches) {
			if (@($selected | Where-Object { $_.name -eq $case.name }).Count -eq 0) {
				$selected += $case
			}
		}
	}
	if ($effectivePatternCount -eq 0) {
		throw "SVG compare case filter is empty."
	}
	Assert-XgeSvgCompareCasesSelected -Cases $selected
	return $selected
}

function Write-XgeSvgCompareCaseList {
	param(
		[object[]]$Cases
	)

	foreach ($case in $Cases) {
		$kind = if ($case.Contains("experimental") -and $case.experimental) { "experimental" } else { "stable" }
		$tags = (Get-XgeSvgCompareCaseTags -Case $case) -join ","
		Write-Output ("{0}`t{1}`t{2}`t{3}" -f $case.name, $kind, $case.svg, $tags)
	}
}
