function Get-XgeSvgCompareCases {
	param(
		[switch]$IncludeExperimental
	)

	$cases = @(
		[ordered]@{ name = "shapes"; svg = "examples\xge_svg\assets\shapes.svg" },
		[ordered]@{ name = "compat"; svg = "examples\xge_svg\assets\compat.svg" },
		[ordered]@{ name = "mask_text_image"; svg = "examples\xge_svg\assets\mask_text_image.svg" },
		[ordered]@{ name = "clip_vector"; svg = "examples\xge_svg\assets\clip_vector.svg" },
		[ordered]@{ name = "clip_mixed_union"; svg = "examples\xge_svg\assets\clip_mixed_union.svg" },
		[ordered]@{ name = "clip_multi_mixed_union"; svg = "examples\xge_svg\assets\clip_multi_mixed_union.svg" },
		[ordered]@{ name = "clip_nested_clip_path"; svg = "examples\xge_svg\assets\clip_nested_clip_path.svg" },
		[ordered]@{ name = "mask_vector"; svg = "examples\xge_svg\assets\mask_vector.svg" },
		[ordered]@{ name = "mask_group_style"; svg = "examples\xge_svg\assets\mask_group_style.svg" },
		[ordered]@{ name = "clip_mask_use"; svg = "examples\xge_svg\assets\clip_mask_use.svg" },
		[ordered]@{ name = "mask_region"; svg = "examples\xge_svg\assets\mask_region.svg" },
		[ordered]@{ name = "mask_stroke"; svg = "examples\xge_svg\assets\mask_stroke.svg" },
		[ordered]@{ name = "gradient_cross_href"; svg = "examples\xge_svg\assets\gradient_cross_href.svg" },
		[ordered]@{ name = "gradient_units"; svg = "examples\xge_svg\assets\gradient_units.svg"; tags = @("shapeex-linear-gpu-batch-43") },
		[ordered]@{ name = "gradient_stop_order"; svg = "examples\xge_svg\assets\gradient_stop_order.svg"; tags = @("gradient-batch-2") },
		[ordered]@{ name = "gradient_spread_repeat"; svg = "examples\xge_svg\assets\gradient_spread_repeat.svg"; tags = @("gradient-batch-2", "shapeex-gradient-batch-39", "shapeex-linear-gpu-batch-43") },
		[ordered]@{ name = "gradient_radial_fr"; svg = "examples\xge_svg\assets\gradient_radial_fr.svg" },
		[ordered]@{ name = "shape_path_order"; svg = "examples\xge_svg\assets\shape_path_order.svg" },
		[ordered]@{ name = "points_prefix"; svg = "examples\xge_svg\assets\points_prefix.svg" },
		[ordered]@{ name = "path_prefix"; svg = "examples\xge_svg\assets\path_prefix.svg" },
		[ordered]@{ name = "path_number_grammar"; svg = "examples\xge_svg\assets\path_number_grammar.svg" },
		[ordered]@{ name = "path_quadratic"; svg = "examples\xge_svg\assets\path_quadratic.svg" },
		[ordered]@{ name = "number_grammar_attrs"; svg = "examples\xge_svg\assets\number_grammar_attrs.svg" },
		[ordered]@{ name = "circle_ellipse_attrs"; svg = "examples\xge_svg\assets\circle_ellipse_attrs.svg" },
		[ordered]@{ name = "path_length"; svg = "examples\xge_svg\assets\path_length.svg" },
		[ordered]@{ name = "stroke_dasharray_odd"; svg = "examples\xge_svg\assets\stroke_dasharray_odd.svg" },
		[ordered]@{ name = "stroke_dash_join"; svg = "examples\xge_svg\assets\stroke_dash_join.svg" },
		[ordered]@{ name = "stroke_linejoin"; svg = "examples\xge_svg\assets\stroke_linejoin.svg" },
		[ordered]@{ name = "stroke_zero_length_caps"; svg = "examples\xge_svg\assets\stroke_zero_length_caps.svg" },
		[ordered]@{ name = "stroke_miterlimit"; svg = "examples\xge_svg\assets\stroke_miterlimit.svg" },
		[ordered]@{ name = "stroke_miterlimit_invalid"; svg = "examples\xge_svg\assets\stroke_miterlimit_invalid.svg"; tags = @("stroke-batch-2", "svg-stroke-batch-38") },
		[ordered]@{ name = "stroke_roundrect"; svg = "examples\xge_svg\assets\stroke_roundrect.svg" },
		[ordered]@{ name = "pattern_units"; svg = "examples\xge_svg\assets\pattern_units.svg" },
		[ordered]@{ name = "pattern_stroke"; svg = "examples\xge_svg\assets\pattern_stroke.svg" },
		[ordered]@{ name = "pattern_overflow"; svg = "examples\xge_svg\assets\pattern_overflow.svg" },
		[ordered]@{ name = "group_use"; svg = "examples\xge_svg\assets\group_use.svg" },
		[ordered]@{ name = "svg_use"; svg = "examples\xge_svg\assets\svg_use.svg" },
		[ordered]@{ name = "nested_svg_viewport_clip"; svg = "examples\xge_svg\assets\nested_svg_viewport_clip.svg" },
		[ordered]@{ name = "rect_invalid_radii"; svg = "examples\xge_svg\assets\rect_invalid_radii.svg" },
		[ordered]@{ name = "blend_group_mask_isolation"; svg = "examples\xge_svg\assets\blend_group_mask_isolation.svg"; tags = @("blend-mode-batch-3", "shapeex-blend-batch-40", "blend", "group", "mask") }
	)
	if ($IncludeExperimental) {
		$cases += [ordered]@{ name = "clip_rule_ignored"; svg = "examples\xge_svg\assets\clip_rule.svg"; tags = @("presentation-container-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "clip_use_rule_ignored"; svg = "examples\xge_svg\assets\clip_use_rule.svg"; tags = @("presentation-container-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "mask_text_image_vector"; svg = "examples\xge_svg\assets\mask_text_image_vector.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_offset"; svg = "examples\xge_svg\assets\filter_offset.svg"; tags = @("filter-shadow-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_drop_shadow"; svg = "examples\xge_svg\assets\filter_drop_shadow.svg"; tags = @("filter-shadow-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_drop_shadow_primitive"; svg = "examples\xge_svg\assets\filter_drop_shadow_primitive.svg"; tags = @("filter-shadow-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_drop_shadow_primitive_region"; svg = "examples\xge_svg\assets\filter_drop_shadow_primitive_region.svg"; tags = @("filter-shadow-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_region"; svg = "examples\xge_svg\assets\filter_region.svg"; tags = @("filter-core-batch-10"); experimental = $true }
		$cases += [ordered]@{ name = "filter_gaussian_blur"; svg = "examples\xge_svg\assets\filter_gaussian_blur.svg"; tags = @("filter-core-batch-10"); experimental = $true }
		$cases += [ordered]@{ name = "filter_gaussian_multiple"; svg = "examples\xge_svg\assets\filter_gaussian_multiple.svg"; tags = @("filter-core-batch-10"); experimental = $true }
		$cases += [ordered]@{ name = "filter_gaussian_input_result_ignored"; svg = "examples\xge_svg\assets\filter_gaussian_input_result_ignored.svg"; tags = @("filter-core-batch-10"); experimental = $true }
		$cases += [ordered]@{ name = "filter_gaussian_zero"; svg = "examples\xge_svg\assets\filter_gaussian_zero.svg"; tags = @("filter-core-batch-10"); experimental = $true }
		$cases += [ordered]@{ name = "filter_gaussian_user_percent_region"; svg = "examples\xge_svg\assets\filter_gaussian_user_percent_region.svg"; tags = @("filter-core-batch-10"); experimental = $true }
		$cases += [ordered]@{ name = "filter_gaussian_edge_wrap"; svg = "examples\xge_svg\assets\filter_gaussian_edge_wrap.svg"; tags = @("filter-core-batch-10"); experimental = $true }
		$cases += [ordered]@{ name = "filter_gaussian_nested_svg"; svg = "examples\xge_svg\assets\filter_gaussian_nested_svg.svg"; tags = @("image"); experimental = $true }
		$cases += [ordered]@{ name = "filter_gaussian_raster"; svg = "examples\xge_svg\assets\filter_gaussian_raster.svg"; tags = @("filter-core-batch-10", "decode-divergence"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_translate"; svg = "examples\xge_svg\assets\filter_transform_translate.svg"; tags = @("transform", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_scale_uniform"; svg = "examples\xge_svg\assets\filter_transform_scale_uniform.svg"; tags = @("transform", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_scale_nonuniform"; svg = "examples\xge_svg\assets\filter_transform_scale_nonuniform.svg"; tags = @("transform", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_rotate"; svg = "examples\xge_svg\assets\filter_transform_rotate.svg"; tags = @("transform", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_skew"; svg = "examples\xge_svg\assets\filter_transform_skew.svg"; tags = @("transform", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_matrix"; svg = "examples\xge_svg\assets\filter_transform_matrix.svg"; tags = @("transform", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_group_child"; svg = "examples\xge_svg\assets\filter_transform_group_child.svg"; tags = @("transform", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_use"; svg = "examples\xge_svg\assets\filter_transform_use.svg"; tags = @("transform", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_nested_svg"; svg = "examples\xge_svg\assets\filter_transform_nested_svg.svg"; tags = @("transform", "image", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_transform_group_composite"; svg = "examples\xge_svg\assets\filter_transform_group_composite.svg"; tags = @("transform", "group", "filter-transform-batch-26"); experimental = $true }
		$cases += [ordered]@{ name = "filter_forward_shape"; svg = "examples\xge_svg\assets\filter_forward_shape.svg"; tags = @("forward-ref"); experimental = $true }
		$cases += [ordered]@{ name = "filter_forward_group"; svg = "examples\xge_svg\assets\filter_forward_group.svg"; tags = @("forward-ref", "group"); experimental = $true }
		$cases += [ordered]@{ name = "filter_def_shape_use"; svg = "examples\xge_svg\assets\filter_def_shape_use.svg"; tags = @("defs", "use"); experimental = $true }
		$cases += [ordered]@{ name = "filter_def_group_use"; svg = "examples\xge_svg\assets\filter_def_group_use.svg"; tags = @("defs", "use", "group"); experimental = $true }
		$cases += [ordered]@{ name = "filter_use_override"; svg = "examples\xge_svg\assets\filter_use_override.svg"; tags = @("defs", "use"); experimental = $true }
		$cases += [ordered]@{ name = "filter_object_bbox_units"; svg = "examples\xge_svg\assets\filter_object_bbox_units.svg"; tags = @("object-bbox"); experimental = $true }
		$cases += [ordered]@{ name = "filter_object_bbox_transform"; svg = "examples\xge_svg\assets\filter_object_bbox_transform.svg"; tags = @("object-bbox", "transform"); experimental = $true }
		$cases += [ordered]@{ name = "filter_primitive_region_object_bbox"; svg = "examples\xge_svg\assets\filter_primitive_region_object_bbox.svg"; tags = @("object-bbox", "primitive-region"); experimental = $true }
		$cases += [ordered]@{ name = "filter_nested_groups"; svg = "examples\xge_svg\assets\filter_nested_groups.svg"; tags = @("group"); experimental = $true }
		$cases += [ordered]@{ name = "filter_group_zero_region"; svg = "examples\xge_svg\assets\filter_group_zero_region.svg"; tags = @("group", "primitive-region"); experimental = $true }
		$cases += [ordered]@{ name = "filter_offset_blur"; svg = "examples\xge_svg\assets\filter_offset_blur.svg"; tags = @("filter-shadow-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_offset_chain"; svg = "examples\xge_svg\assets\filter_offset_chain.svg"; tags = @("filter-shadow-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_offset_source_alpha"; svg = "examples\xge_svg\assets\filter_offset_source_alpha.svg"; tags = @("filter-shadow-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_offset_primitive_region"; svg = "examples\xge_svg\assets\filter_offset_primitive_region.svg"; tags = @("filter-shadow-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_blur_source_alpha"; svg = "examples\xge_svg\assets\filter_blur_source_alpha.svg"; tags = @("filter-shadow-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_blur_primitive_region"; svg = "examples\xge_svg\assets\filter_blur_primitive_region.svg"; tags = @("filter-shadow-batch-1", "svg-filter-region-batch-55"); experimental = $true }
		$cases += [ordered]@{ name = "filter_blur_primitive_region_wrap"; svg = "examples\xge_svg\assets\filter_blur_primitive_region_wrap.svg"; tags = @("filter-core-batch-10", "svg-filter-region-batch-55"); experimental = $true }
		$cases += [ordered]@{ name = "filter_color_matrix"; svg = "examples\xge_svg\assets\filter_color_matrix.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_component_transfer"; svg = "examples\xge_svg\assets\filter_component_transfer.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_flood"; svg = "examples\xge_svg\assets\filter_flood.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_current_color"; svg = "examples\xge_svg\assets\filter_current_color.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_shadow_pipeline"; svg = "examples\xge_svg\assets\filter_shadow_pipeline.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_blend"; svg = "examples\xge_svg\assets\filter_blend.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_blend_normal_source_media"; svg = "examples\xge_svg\assets\filter_blend_normal_source_media.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_color_graph"; svg = "examples\xge_svg\assets\filter_color_graph.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_color_graph_multi"; svg = "examples\xge_svg\assets\filter_color_graph_multi.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_independent_color_graph"; svg = "examples\xge_svg\assets\filter_independent_color_graph.svg"; tags = @("filter-color-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_identity_graph"; svg = "examples\xge_svg\assets\filter_identity_graph.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_displacement_zero"; svg = "examples\xge_svg\assets\filter_displacement_zero.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_displacement_shift"; svg = "examples\xge_svg\assets\filter_displacement_shift.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_convolve_identity"; svg = "examples\xge_svg\assets\filter_convolve_identity.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_convolve_solid"; svg = "examples\xge_svg\assets\filter_convolve_solid.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_identity"; svg = "examples\xge_svg\assets\filter_morphology_identity.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_independent"; svg = "examples\xge_svg\assets\filter_morphology_independent.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_region"; svg = "examples\xge_svg\assets\filter_morphology_region.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_source_alpha"; svg = "examples\xge_svg\assets\filter_morphology_source_alpha.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_source_erode"; svg = "examples\xge_svg\assets\filter_morphology_source_erode.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_ellipse_erode"; svg = "examples\xge_svg\assets\filter_morphology_ellipse_erode.svg"; tags = @("filter-raster-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_morphology_primitive_region"; svg = "examples\xge_svg\assets\filter_morphology_primitive_region.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_tile_independent"; svg = "examples\xge_svg\assets\filter_tile_independent.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_tile_source"; svg = "examples\xge_svg\assets\filter_tile_source.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_image"; svg = "examples\xge_svg\assets\filter_image.svg"; tags = @("filter-core-batch-10"); experimental = $true }
		$cases += [ordered]@{ name = "filter_image_data_fragment"; svg = "examples\xge_svg\assets\filter_image_data_fragment.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_image_local_ref"; svg = "examples\xge_svg\assets\filter_image_local_ref.svg"; tags = @("filter-core-batch-10"); experimental = $true }
		$cases += [ordered]@{ name = "filter_image_url_fragment"; svg = "examples\xge_svg\assets\filter_image_url_fragment.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_primitive_region"; svg = "examples\xge_svg\assets\filter_primitive_region.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_offset_independent_region"; svg = "examples\xge_svg\assets\filter_offset_independent_region.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_composite_region"; svg = "examples\xge_svg\assets\filter_composite_region.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_composite_over_text"; svg = "examples\xge_svg\assets\filter_composite_over_text.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_composite_source_clip_media"; svg = "examples\xge_svg\assets\filter_composite_source_clip_media.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_composite"; svg = "examples\xge_svg\assets\filter_composite.svg"; tags = @("remaining-core-batch-11"); experimental = $true }
		$cases += [ordered]@{ name = "filter_merge"; svg = "examples\xge_svg\assets\filter_merge.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_merge_source_media"; svg = "examples\xge_svg\assets\filter_merge_source_media.svg"; tags = @("filter-region-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "filter_units_invalid"; svg = "examples\xge_svg\assets\filter_units_invalid.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "raster_image"; svg = "examples\xge_svg\assets\raster_image.svg"; tags = @("remaining-core-batch-11", "decode-divergence"); experimental = $true }
		$cases += [ordered]@{ name = "image_data_uri_policy"; svg = "examples\xge_svg\assets\image_data_uri_policy.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "image_svg_data_charset"; svg = "examples\xge_svg\assets\image_svg_data_charset.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "image_svg_data_fragment"; svg = "examples\xge_svg\assets\image_svg_data_fragment.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "image_jpg_data_uri"; svg = "examples\xge_svg\assets\image_jpg_data_uri.svg"; tags = @("remaining-core-batch-11", "decode-divergence"); experimental = $true }
		$cases += [ordered]@{ name = "external_image"; svg = "examples\xge_svg\assets\external_image.svg"; tags = @("remaining-core-batch-11", "decode-divergence"); experimental = $true }
		$cases += [ordered]@{ name = "external_use"; svg = "examples\xge_svg\assets\external_use.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "use_data_uri_fragment"; svg = "examples\xge_svg\assets\use_data_uri_fragment.svg"; tags = @("resource-reference-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "use_external_root_ignored"; svg = "examples\xge_svg\assets\use_external_root_ignored.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "use_external_fragment_ignored"; svg = "examples\xge_svg\assets\use_external_fragment_ignored.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "use_external_url_ignored"; svg = "examples\xge_svg\assets\use_external_url_ignored.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "use_data_uri_ignored"; svg = "examples\xge_svg\assets\use_data_uri_ignored.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "use_local_bare_id"; svg = "examples\xge_svg\assets\use_local_bare_id.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "use_forward_reference"; svg = "examples\xge_svg\assets\use_forward_reference.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "use_forward_paint_order"; svg = "examples\xge_svg\assets\use_forward_paint_order.svg"; tags = @("use-batch", "svg-use-order-batch-53", "paint-order", "forward-ref"); experimental = $true }
		$cases += [ordered]@{ name = "use_forward_filter_order"; svg = "examples\xge_svg\assets\use_forward_filter_order.svg"; tags = @("use-batch", "svg-use-order-batch-53", "filter", "forward-ref"); experimental = $true }
		$cases += [ordered]@{ name = "use_descendant_element_transform"; svg = "examples\xge_svg\assets\use_descendant_element_transform.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "use_descendant_group_transform"; svg = "examples\xge_svg\assets\use_descendant_group_transform.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "use_symbol_default_clip"; svg = "examples\xge_svg\assets\use_symbol_default_clip.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "use_symbol_overflow_visible"; svg = "examples\xge_svg\assets\use_symbol_overflow_visible.svg"; tags = @("use-batch"); experimental = $true }
		$cases += [ordered]@{ name = "marker_arc_mid_vertex"; svg = "examples\xge_svg\assets\marker_arc_mid_vertex.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "marker_bounds_hit"; svg = "examples\xge_svg\assets\marker_bounds_hit.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "marker_closed_path"; svg = "examples\xge_svg\assets\marker_closed_path.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "marker_invalid_reference"; svg = "examples\xge_svg\assets\marker_invalid_reference.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "marker_mixed_content"; svg = "examples\xge_svg\assets\marker_mixed_content.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "marker_orient_angle_units"; svg = "examples\xge_svg\assets\marker_orient_angle_units.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "marker_overflow_clip"; svg = "examples\xge_svg\assets\marker_overflow_clip.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "marker_quadratic_path"; svg = "examples\xge_svg\assets\marker_quadratic_path.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "marker_ref_keywords"; svg = "examples\xge_svg\assets\marker_ref_keywords.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "marker_units"; svg = "examples\xge_svg\assets\marker_units.svg"; tags = @("marker-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_offset_invalid"; svg = "examples\xge_svg\assets\gradient_stop_offset_invalid.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_style_order"; svg = "examples\xge_svg\assets\gradient_stop_style_order.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_case"; svg = "examples\xge_svg\assets\gradient_stop_case.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_equal_offset"; svg = "examples\xge_svg\assets\gradient_equal_offset.svg"; tags = @("gradient-batch-2", "shapeex-linear-gpu-batch-43"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_single_stop"; svg = "examples\xge_svg\assets\gradient_single_stop.svg"; tags = @("gradient-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_linear_object_bbox"; svg = "examples\xge_svg\assets\gradient_linear_object_bbox.svg"; tags = @("gradient-batch-2", "shapeex-linear-gpu-batch-43"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_linear_user_space"; svg = "examples\xge_svg\assets\gradient_linear_user_space.svg"; tags = @("gradient-batch-2", "shapeex-linear-gpu-batch-43"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_radial_object_bbox"; svg = "examples\xge_svg\assets\gradient_radial_object_bbox.svg"; tags = @("gradient-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_radial_user_space"; svg = "examples\xge_svg\assets\gradient_radial_user_space.svg"; tags = @("gradient-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_transform_order"; svg = "examples\xge_svg\assets\gradient_transform_order.svg"; tags = @("gradient-batch-2", "shapeex-linear-gpu-batch-43"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_spread_reflect"; svg = "examples\xge_svg\assets\gradient_spread_reflect.svg"; tags = @("gradient-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_element_font_size"; svg = "examples\xge_svg\assets\gradient_element_font_size.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_element_color"; svg = "examples\xge_svg\assets\gradient_element_color.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_parent_current_color"; svg = "examples\xge_svg\assets\gradient_parent_current_color.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_radial_zero_radius"; svg = "examples\xge_svg\assets\gradient_radial_zero_radius.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_radial_fr_fill"; svg = "examples\xge_svg\assets\gradient_radial_fr_fill.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_radial_fr_stroke"; svg = "examples\xge_svg\assets\gradient_radial_fr_stroke.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_radial_focal_outside"; svg = "examples\xge_svg\assets\gradient_radial_focal_outside.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_radial_fr_oversize"; svg = "examples\xge_svg\assets\gradient_radial_fr_oversize.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_href_units_linear"; svg = "examples\xge_svg\assets\gradient_href_units_linear.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_href_units_radial"; svg = "examples\xge_svg\assets\gradient_href_units_radial.svg"; tags = @("gradient-batch-3"); experimental = $true }
		$cases += [ordered]@{ name = "radial_gradient_gpu_batch_41"; svg = "examples\xge_svg\assets\radial_gradient_gpu_batch_41.svg"; tags = @("shapeex-radial-gpu-batch-41", "gradient", "stroke", "mask", "blend", "transform"); experimental = $true }
		$cases += [ordered]@{ name = "enum_inherit_invalid"; svg = "examples\xge_svg\assets\enum_inherit_invalid.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "style_invalid_keywords"; svg = "examples\xge_svg\assets\style_invalid_keywords.svg"; tags = @("loader-invalid-batch-5"); experimental = $true }
		$cases += [ordered]@{ name = "style_duplicate_invalid"; svg = "examples\xge_svg\assets\style_duplicate_invalid.svg"; tags = @("loader-invalid-batch-5"); experimental = $true }
		$cases += [ordered]@{ name = "text_style_duplicate_invalid"; svg = "examples\xge_svg\assets\text_style_duplicate_invalid.svg"; tags = @("loader-invalid-batch-5", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "visibility_ignored"; svg = "examples\xge_svg\assets\visibility_ignored.svg"; tags = @("presentation-container-batch-1", "loader-invalid-batch-5"); experimental = $true }
		$cases += [ordered]@{ name = "vector_effect_ignored"; svg = "examples\xge_svg\assets\vector_effect_ignored.svg"; tags = @("presentation-container-batch-1", "loader-invalid-batch-5"); experimental = $true }
		$cases += [ordered]@{ name = "text_decoration_ignored"; svg = "examples\xge_svg\assets\text_decoration_ignored.svg"; tags = @("loader-invalid-batch-5", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "color_function_grammar"; svg = "examples\xge_svg\assets\color_function_grammar.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "hsl_grammar"; svg = "examples\xge_svg\assets\hsl_grammar.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "css_property_case"; svg = "examples\xge_svg\assets\css_property_case.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "css_keyword_case"; svg = "examples\xge_svg\assets\css_keyword_case.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "css_attribute_case_flag"; svg = "examples\xge_svg\assets\css_attribute_case_flag.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "css_identifier_escape"; svg = "examples\xge_svg\assets\css_identifier_escape.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "css_attribute_escape"; svg = "examples\xge_svg\assets\css_attribute_escape.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "css_url_escape"; svg = "examples\xge_svg\assets\css_url_escape.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "css_nth_spacing"; svg = "examples\xge_svg\assets\css_nth_spacing.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "css_pseudo_case"; svg = "examples\xge_svg\assets\css_pseudo_case.svg"; tags = @("css-grammar-batch-7"); experimental = $true }
		$cases += [ordered]@{ name = "css_media_rule"; svg = "examples\xge_svg\assets\css_media_rule.svg"; tags = @("remaining-core-batch-11"); experimental = $true }
		$cases += [ordered]@{ name = "css_thorvg_subset"; svg = "examples\xge_svg\assets\css_thorvg_subset.svg"; tags = @("remaining-core-batch-11"); experimental = $true }
		$cases += [ordered]@{ name = "style_first_only"; svg = "examples\xge_svg\assets\style_first_only.svg"; tags = @("remaining-core-batch-11"); experimental = $true }
		$cases += [ordered]@{ name = "opacity_invalid"; svg = "examples\xge_svg\assets\opacity_invalid.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "opacity_duplicate_invalid"; svg = "examples\xge_svg\assets\opacity_duplicate_invalid.svg"; tags = @("loader-invalid-batch-5"); experimental = $true }
		$cases += [ordered]@{ name = "mix_blend_mode"; svg = "examples\xge_svg\assets\mix_blend_mode.svg"; tags = @("presentation-container-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_multiply"; svg = "examples\xge_svg\assets\blend_mode_multiply.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_screen"; svg = "examples\xge_svg\assets\blend_mode_screen.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_darken"; svg = "examples\xge_svg\assets\blend_mode_darken.svg"; tags = @("blend-mode-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_lighten"; svg = "examples\xge_svg\assets\blend_mode_lighten.svg"; tags = @("blend-mode-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_overlay"; svg = "examples\xge_svg\assets\blend_mode_overlay.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_color_dodge"; svg = "examples\xge_svg\assets\blend_mode_color_dodge.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_color_burn"; svg = "examples\xge_svg\assets\blend_mode_color_burn.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_hard_light"; svg = "examples\xge_svg\assets\blend_mode_hard_light.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_soft_light"; svg = "examples\xge_svg\assets\blend_mode_soft_light.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_difference"; svg = "examples\xge_svg\assets\blend_mode_difference.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_exclusion"; svg = "examples\xge_svg\assets\blend_mode_exclusion.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_hue"; svg = "examples\xge_svg\assets\blend_mode_hue.svg"; tags = @("blend-mode-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_saturation"; svg = "examples\xge_svg\assets\blend_mode_saturation.svg"; tags = @("blend-mode-batch-1", "shapeex-blend-8bit-batch-44"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_color"; svg = "examples\xge_svg\assets\blend_mode_color.svg"; tags = @("blend-mode-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mode_luminosity"; svg = "examples\xge_svg\assets\blend_mode_luminosity.svg"; tags = @("blend-mode-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "blend_group"; svg = "examples\xge_svg\assets\blend_group.svg"; tags = @("blend-mode-batch-2", "blend", "group"); experimental = $true }
		$cases += [ordered]@{ name = "blend_group_opacity"; svg = "examples\xge_svg\assets\blend_group_opacity.svg"; tags = @("blend-mode-batch-2", "blend", "group", "opacity"); experimental = $true }
		$cases += [ordered]@{ name = "blend_nested_groups"; svg = "examples\xge_svg\assets\blend_nested_groups.svg"; tags = @("blend-mode-batch-2", "blend", "group", "nested"); experimental = $true }
		$cases += [ordered]@{ name = "blend_linear_gradient"; svg = "examples\xge_svg\assets\blend_linear_gradient.svg"; tags = @("blend-mode-batch-2", "blend", "gradient"); experimental = $true }
		$cases += [ordered]@{ name = "blend_radial_gradient"; svg = "examples\xge_svg\assets\blend_radial_gradient.svg"; tags = @("blend-mode-batch-2", "blend", "gradient"); experimental = $true }
		$cases += [ordered]@{ name = "blend_stroke"; svg = "examples\xge_svg\assets\blend_stroke.svg"; tags = @("blend-mode-batch-2", "blend", "stroke"); experimental = $true }
		$cases += [ordered]@{ name = "blend_clip"; svg = "examples\xge_svg\assets\blend_clip.svg"; tags = @("blend-mode-batch-2", "blend", "clip"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mask"; svg = "examples\xge_svg\assets\blend_mask.svg"; tags = @("blend-mode-batch-2", "blend", "mask"); experimental = $true }
		$cases += [ordered]@{ name = "blend_transform"; svg = "examples\xge_svg\assets\blend_transform.svg"; tags = @("blend-mode-batch-2", "blend", "transform"); experimental = $true }
		$cases += [ordered]@{ name = "blend_use"; svg = "examples\xge_svg\assets\blend_use.svg"; tags = @("blend-mode-batch-2", "blend", "use"); experimental = $true }
		$cases += [ordered]@{ name = "blend_group_clip_isolation"; svg = "examples\xge_svg\assets\blend_group_clip_isolation.svg"; tags = @("blend-mode-batch-3", "blend", "group", "clip"); experimental = $true }
		$cases += [ordered]@{ name = "blend_mask_opacity_batch_40"; svg = "examples\xge_svg\assets\blend_mask_opacity_batch_40.svg"; tags = @("shapeex-blend-batch-40", "blend", "group", "mask", "opacity", "luminance", "alpha", "nested"); experimental = $true }
		$cases += [ordered]@{ name = "mask_shapeex_scene_batch_30"; svg = "examples\xge_svg\assets\mask_shapeex_scene_batch_30.svg"; tags = @("shapeex-mask-batch-30", "mask", "shape", "scene", "use"); experimental = $true }
		$cases += [ordered]@{ name = "mask_external_scene_batch_31"; svg = "examples\xge_svg\assets\mask_external_scene_batch_31.svg"; tags = @("shapeex-mask-batch-31", "mask", "text", "image", "raster", "use", "decode-divergence"); experimental = $true }
		$cases += [ordered]@{ name = "mask_luminance_batch_32"; svg = "examples\xge_svg\assets\mask_luminance_batch_32.svg"; tags = @("shapeex-mask-batch-32", "mask", "luminance", "gradient", "text", "image", "raster", "use", "decode-divergence"); experimental = $true }
		$cases += [ordered]@{ name = "mask_gradient_batch_33"; svg = "examples\xge_svg\assets\mask_gradient_batch_33.svg"; tags = @("shapeex-mask-batch-33", "mask", "luminance", "alpha", "gradient", "stroke", "transform"); experimental = $true }
		$cases += [ordered]@{ name = "mask_nested_svg_viewport_batch_34"; svg = "examples\xge_svg\assets\mask_nested_svg_viewport_batch_34.svg"; tags = @("shapeex-mask-batch-34", "mask", "image", "nested-svg", "viewport", "aspect", "transform"); experimental = $true }
		$cases += [ordered]@{ name = "image_transform_stroke_batch_35"; svg = "examples\xge_svg\assets\image_transform_stroke_batch_35.svg"; tags = @("shapeex-mask-batch-35", "image", "nested-svg", "stroke", "transform", "mask"); experimental = $true }
		$cases += [ordered]@{ name = "image_source_batch_36"; svg = "examples\xge_svg\assets\image_source_batch_36.svg"; tags = @("image-source-batch-36", "image", "external", "data-uri", "resource"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_miter_turn_batch_37"; svg = "examples\xge_svg\assets\stroke_miter_turn_batch_37.svg"; tags = @("shapeex-stroke-batch-37", "stroke", "miter", "join", "bounds", "hit-test"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_reverse_join"; svg = "examples\xge_svg\assets\stroke_reverse_join.svg"; tags = @("shapeex-stroke-batch-51", "stroke", "join", "reverse"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_miter_invalid_batch_38"; svg = "examples\xge_svg\assets\stroke_miter_invalid_batch_38.svg"; tags = @("svg-stroke-batch-38", "stroke", "miter", "invalid", "inheritance", "style"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_repeat_seam_batch_39"; svg = "examples\xge_svg\assets\gradient_repeat_seam_batch_39.svg"; tags = @("shapeex-gradient-batch-39", "shapeex-linear-gpu-batch-43", "gradient", "repeat", "seam", "stroke", "radial", "transform"); experimental = $true }
		$cases += [ordered]@{ name = "blend_group_filter_order"; svg = "examples\xge_svg\assets\blend_group_filter_order.svg"; tags = @("blend-mode-batch-3", "blend", "group", "filter"); experimental = $true }
		$cases += [ordered]@{ name = "blend_root_opacity"; svg = "examples\xge_svg\assets\blend_root_opacity.svg"; tags = @("blend-mode-batch-3", "blend", "root", "opacity"); experimental = $true }
		$cases += [ordered]@{ name = "blend_nested_svg_opacity"; svg = "examples\xge_svg\assets\blend_nested_svg_opacity.svg"; tags = @("blend-mode-batch-3", "blend", "svg", "opacity"); experimental = $true }
		$cases += [ordered]@{ name = "blend_nested_svg_mode"; svg = "examples\xge_svg\assets\blend_nested_svg_mode.svg"; tags = @("blend-mode-batch-3", "blend", "svg", "nested"); experimental = $true }
		$cases += [ordered]@{ name = "blend_defs_nested_group_use"; svg = "examples\xge_svg\assets\blend_defs_nested_group_use.svg"; tags = @("blend-mode-batch-3", "svg-def-group-batch-57", "blend", "defs", "group", "use"); experimental = $true }
		$cases += [ordered]@{ name = "defs_group_opacity_use"; svg = "examples\xge_svg\assets\defs_group_opacity_use.svg"; tags = @("svg-def-group-batch-57", "defs", "group", "opacity", "use"); experimental = $true }
		$cases += [ordered]@{ name = "defs_group_sibling_isolation_use"; svg = "examples\xge_svg\assets\defs_group_sibling_isolation_use.svg"; tags = @("svg-def-group-batch-57", "blend", "defs", "group", "opacity", "use"); experimental = $true }
		$cases += [ordered]@{ name = "use_forward_nested_group_isolation"; svg = "examples\xge_svg\assets\use_forward_nested_group_isolation.svg"; tags = @("svg-def-group-batch-57", "blend", "defs", "forward-ref", "group", "use"); experimental = $true }
		$cases += [ordered]@{ name = "blend_fill_gradient_stroke_solid"; svg = "examples\xge_svg\assets\blend_fill_gradient_stroke_solid.svg"; tags = @("blend-mode-batch-3", "shapeex-linear-gpu-batch-43", "shapeex-stroke-batch-54", "blend", "gradient", "stroke"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_transform_nonuniform_blend"; svg = "examples\xge_svg\assets\stroke_transform_nonuniform_blend.svg"; tags = @("shapeex-stroke-batch-54", "stroke", "transform", "blend", "gradient", "bounds"); experimental = $true }
		$cases += [ordered]@{ name = "blend_fill_solid_stroke_gradient"; svg = "examples\xge_svg\assets\blend_fill_solid_stroke_gradient.svg"; tags = @("blend-mode-batch-3", "shapeex-linear-gpu-batch-43", "blend", "gradient", "stroke"); experimental = $true }
		$cases += [ordered]@{ name = "blend_fill_stroke_gradients"; svg = "examples\xge_svg\assets\blend_fill_stroke_gradients.svg"; tags = @("blend-mode-batch-3", "blend", "gradient", "stroke", "paint-order"); experimental = $true }
		$cases += [ordered]@{ name = "fill_self_intersect_nonzero"; svg = "examples\xge_svg\assets\fill_self_intersect_nonzero.svg"; tags = @("fill-rule-batch-4", "fill", "path", "self-intersect", "nonzero"); experimental = $true }
		$cases += [ordered]@{ name = "fill_self_intersect_evenodd"; svg = "examples\xge_svg\assets\fill_self_intersect_evenodd.svg"; tags = @("fill-rule-batch-4", "fill", "path", "self-intersect", "evenodd"); experimental = $true }
		$cases += [ordered]@{ name = "fill_bowtie_nonzero"; svg = "examples\xge_svg\assets\fill_bowtie_nonzero.svg"; tags = @("fill-rule-batch-4", "fill", "path", "self-intersect", "nonzero"); experimental = $true }
		$cases += [ordered]@{ name = "fill_open_implicit_close"; svg = "examples\xge_svg\assets\fill_open_implicit_close.svg"; tags = @("fill-rule-batch-4", "fill", "path", "open"); experimental = $true }
		$cases += [ordered]@{ name = "fill_compound_same_winding"; svg = "examples\xge_svg\assets\fill_compound_same_winding.svg"; tags = @("fill-rule-batch-4", "fill", "path", "compound", "nonzero"); experimental = $true }
		$cases += [ordered]@{ name = "fill_compound_opposite_winding"; svg = "examples\xge_svg\assets\fill_compound_opposite_winding.svg"; tags = @("fill-rule-batch-4", "fill", "path", "compound", "nonzero"); experimental = $true }
		$cases += [ordered]@{ name = "fill_compound_three_level_evenodd"; svg = "examples\xge_svg\assets\fill_compound_three_level_evenodd.svg"; tags = @("fill-rule-batch-4", "fill", "path", "compound", "evenodd"); experimental = $true }
		$cases += [ordered]@{ name = "fill_overlapping_subpaths"; svg = "examples\xge_svg\assets\fill_overlapping_subpaths.svg"; tags = @("fill-rule-batch-4", "fill", "path", "compound", "overlap"); experimental = $true }
		$cases += [ordered]@{ name = "fill_transform_gradient_evenodd"; svg = "examples\xge_svg\assets\fill_transform_gradient_evenodd.svg"; tags = @("fill-rule-batch-4", "shapeex-linear-gpu-batch-43", "shapeex-gradient-bbox-batch-56", "fill", "path", "self-intersect", "evenodd", "gradient", "transform"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_object_bbox_shape_transform"; svg = "examples\xge_svg\assets\gradient_object_bbox_shape_transform.svg"; tags = @("shapeex-gradient-bbox-batch-56", "gradient", "transform", "object-bbox"); experimental = $true }
		$cases += [ordered]@{ name = "fill_clip_self_intersect_evenodd"; svg = "examples\xge_svg\assets\fill_clip_self_intersect_evenodd.svg"; tags = @("fill-rule-batch-4", "fill", "path", "self-intersect", "evenodd", "clip"); experimental = $true }
		$cases += [ordered]@{ name = "font_size_invalid"; svg = "examples\xge_svg\assets\font_size_invalid.svg"; tags = @("loader-invalid-batch-5"); experimental = $true }
		$cases += [ordered]@{ name = "font_size_style_ignored"; svg = "examples\xge_svg\assets\font_size_duplicate_invalid.svg"; tags = @("loader-invalid-batch-5", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "text_baseline_ignored"; svg = "examples\xge_svg\assets\text_baseline_shift.svg"; tags = @("loader-invalid-batch-5", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "length_suffix"; svg = "examples\xge_svg\assets\length_suffix.svg"; tags = @("presentation-container-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "xml_markup_boundaries"; svg = "examples\xge_svg\assets\xml_markup_boundaries.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "xml_entities"; svg = "examples\xge_svg\assets\xml_entities.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "paint_order_markers"; svg = "examples\xge_svg\assets\paint_order_markers.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "paint_order_invalid"; svg = "examples\xge_svg\assets\paint_order_invalid.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "paint_invalid"; svg = "examples\xge_svg\assets\paint_invalid.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "paint_invalid_gradient"; svg = "examples\xge_svg\assets\paint_invalid_gradient.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "url_reference_invalid"; svg = "examples\xge_svg\assets\url_reference_invalid.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "url_function_case"; svg = "examples\xge_svg\assets\url_function_case.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "transform_angle_units"; svg = "examples\xge_svg\assets\transform_angle_units.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "use_inherit"; svg = "examples\xge_svg\assets\use_inherit.svg"; tags = @("presentation-container-batch-1"); experimental = $true }
		$cases += [ordered]@{ name = "symbol_overflow"; svg = "examples\xge_svg\assets\symbol_overflow.svg"; tags = @("container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "clip_mask_nested_svg"; svg = "examples\xge_svg\assets\clip_mask_nested_svg.svg"; tags = @("container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "switch_conditional"; svg = "examples\xge_svg\assets\switch_conditional.svg"; tags = @("container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "switch_style_scope"; svg = "examples\xge_svg\assets\switch_style_scope.svg"; tags = @("presentation-container-batch-1", "container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "switch_selector_scope"; svg = "examples\xge_svg\assets\switch_selector_scope.svg"; tags = @("presentation-container-batch-1", "container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "switch_container_inherit"; svg = "examples\xge_svg\assets\switch_container_inherit.svg"; tags = @("presentation-container-batch-1", "container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "conditional_processing_attrs"; svg = "examples\xge_svg\assets\conditional_processing_attrs.svg"; tags = @("container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "anchor_container"; svg = "examples\xge_svg\assets\anchor_container.svg"; tags = @("container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "unsupported_container_children"; svg = "examples\xge_svg\assets\metadata_skip.svg"; tags = @("container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "xlink_href"; svg = "examples\xge_svg\assets\xlink_href.svg"; tags = @("container-conditional-batch-8"); experimental = $true }
		$cases += [ordered]@{ name = "points_invalid"; svg = "examples\xge_svg\assets\points_invalid.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "points_invalid_clip_mask"; svg = "examples\xge_svg\assets\points_invalid_clip_mask.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "path_invalid"; svg = "examples\xge_svg\assets\path_invalid.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "path_incomplete_commands"; svg = "examples\xge_svg\assets\path_incomplete_commands.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "length_number_grammar"; svg = "examples\xge_svg\assets\length_number_grammar.svg"; tags = @("geometry-invalid-batch-9"); experimental = $true }
		$cases += [ordered]@{ name = "path_length_dash_hit"; svg = "examples\xge_svg\assets\path_length_dash_hit.svg"; tags = @("remaining-core-batch-11"); experimental = $true }
		$cases += [ordered]@{ name = "line_path_length_dash_hit"; svg = "examples\xge_svg\assets\line_path_length_dash_hit.svg"; tags = @("remaining-core-batch-11"); experimental = $true }
		$cases += [ordered]@{ name = "preserve_aspect_invalid"; svg = "examples\xge_svg\assets\preserve_aspect_invalid.svg"; tags = @("loader-grammar-batch-6"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_dasharray_invalid"; svg = "examples\xge_svg\assets\stroke_dasharray_invalid.svg"; tags = @("stroke-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_dash_negative"; svg = "examples\xge_svg\assets\stroke_dash_negative.svg"; tags = @("stroke-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_dashoffset_percent"; svg = "examples\xge_svg\assets\stroke_dashoffset_percent.svg"; tags = @("stroke-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_dashoffset_invalid"; svg = "examples\xge_svg\assets\stroke_dashoffset_invalid.svg"; tags = @("stroke-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_duplicate_invalid"; svg = "examples\xge_svg\assets\stroke_duplicate_invalid.svg"; tags = @("stroke-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_dash_inherit"; svg = "examples\xge_svg\assets\stroke_dash_inherit.svg"; tags = @("stroke-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "style_inherit_core"; svg = "examples\xge_svg\assets\style_inherit_core.svg"; tags = @("remaining-core-batch-11"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_line_enum_invalid"; svg = "examples\xge_svg\assets\stroke_line_enum_invalid.svg"; tags = @("stroke-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_miterlimit_negative"; svg = "examples\xge_svg\assets\stroke_miterlimit_negative.svg"; tags = @("stroke-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "stroke_width_invalid"; svg = "examples\xge_svg\assets\stroke_width_invalid.svg"; tags = @("stroke-batch-2"); experimental = $true }
		$cases += [ordered]@{ name = "text_xml_space_default"; svg = "examples\xge_svg\assets\text_xml_space_default.svg"; tags = @("source-edge-batch-12"); experimental = $true }
		$cases += [ordered]@{ name = "text_xml_space_preserve"; svg = "examples\xge_svg\assets\text_xml_space_preserve.svg"; tags = @("source-edge-batch-12"); experimental = $true }
		$cases += [ordered]@{ name = "text_tspan_position"; svg = "examples\xge_svg\assets\text_tspan_position.svg"; tags = @("source-edge-batch-12"); experimental = $true }
		$cases += [ordered]@{ name = "text_tspan_inherit"; svg = "examples\xge_svg\assets\text_tspan_inherit.svg"; tags = @("source-edge-batch-12", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "display_container_semantics"; svg = "examples\xge_svg\assets\display_container_semantics.svg"; tags = @("source-edge-batch-12"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_cascade"; svg = "examples\xge_svg\assets\css_important_cascade.svg"; tags = @("source-edge-batch-12"); experimental = $true }
		$cases += [ordered]@{ name = "use_circular_reference"; svg = "examples\xge_svg\assets\use_circular_reference.svg"; tags = @("source-edge-batch-12"); experimental = $true }
		$cases += [ordered]@{ name = "use_ancestor_reference"; svg = "examples\xge_svg\assets\use_ancestor_reference.svg"; tags = @("source-edge-batch-12"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_stop_outside"; svg = "examples\xge_svg\assets\gradient_stop_outside.svg"; tags = @("source-edge-batch-12"); experimental = $true }
		$cases += [ordered]@{ name = "gradient_nested_definition"; svg = "examples\xge_svg\assets\gradient_nested_definition.svg"; tags = @("source-edge-batch-12"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_ttf_basic"; svg = "examples\xge_svg\assets\font_face_ttf_basic.svg"; tags = @("font-face-batch-13", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_otf_marker"; svg = "examples\xge_svg\assets\font_face_otf_marker.svg"; tags = @("font-face-batch-13", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_inherited"; svg = "examples\xge_svg\assets\font_face_inherited.svg"; tags = @("font-face-batch-13"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_css_class"; svg = "examples\xge_svg\assets\font_face_css_class.svg"; tags = @("font-face-batch-13"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_inline_override"; svg = "examples\xge_svg\assets\font_face_inline_override.svg"; tags = @("font-face-batch-13"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_tspan_inherit"; svg = "examples\xge_svg\assets\font_face_tspan_inherit.svg"; tags = @("font-face-batch-13"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_tspan_override"; svg = "examples\xge_svg\assets\font_face_tspan_override.svg"; tags = @("font-face-batch-13", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_multiple"; svg = "examples\xge_svg\assets\font_face_multiple.svg"; tags = @("font-face-batch-13", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_malformed_src"; svg = "examples\xge_svg\assets\font_face_malformed_src.svg"; tags = @("font-face-batch-13"); experimental = $true }
		$cases += [ordered]@{ name = "font_face_missing_family"; svg = "examples\xge_svg\assets\font_face_missing_family.svg"; tags = @("font-face-batch-13"); experimental = $true }
		$cases += [ordered]@{ name = "text_font_size_style_ignored"; svg = "examples\xge_svg\assets\text_font_size_style_ignored.svg"; tags = @("text-source-batch-14"); experimental = $true }
		$cases += [ordered]@{ name = "text_font_family_style_ignored"; svg = "examples\xge_svg\assets\text_font_family_style_ignored.svg"; tags = @("text-source-batch-14"); experimental = $true }
		$cases += [ordered]@{ name = "text_font_size_keywords_zero"; svg = "examples\xge_svg\assets\text_font_size_keywords_zero.svg"; tags = @("text-source-batch-14"); experimental = $true }
		$cases += [ordered]@{ name = "text_font_size_negative_tspan_fallback"; svg = "examples\xge_svg\assets\text_font_size_negative_tspan_fallback.svg"; tags = @("text-source-batch-14"); experimental = $true }
		$cases += [ordered]@{ name = "text_nontext_font_size_ignored"; svg = "examples\xge_svg\assets\text_nontext_font_size_ignored.svg"; tags = @("text-source-batch-14"); experimental = $true }
		$cases += [ordered]@{ name = "text_tspan_font_family_inherit"; svg = "examples\xge_svg\assets\text_tspan_font_family_inherit.svg"; tags = @("text-source-batch-14", "shapeex-text-aa-batch-42"); experimental = $true }
		$cases += [ordered]@{ name = "text_xml_space_style_ignored"; svg = "examples\xge_svg\assets\text_xml_space_style_ignored.svg"; tags = @("text-source-batch-14"); experimental = $true }
		$cases += [ordered]@{ name = "text_length_spacing_ignored"; svg = "examples\xge_svg\assets\text_length_spacing_ignored.svg"; tags = @("text-source-batch-14"); experimental = $true }
		$cases += [ordered]@{ name = "text_length_glyphs_ignored"; svg = "examples\xge_svg\assets\text_length_glyphs_ignored.svg"; tags = @("text-source-batch-14"); experimental = $true }
		$cases += [ordered]@{ name = "mask_type_upper_attr"; svg = "examples\xge_svg\assets\mask_type_upper_attr.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "mask_type_upper_style"; svg = "examples\xge_svg\assets\mask_type_upper_style.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "mask_type_upper_class"; svg = "examples\xge_svg\assets\mask_type_upper_class.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "mask_type_lowercase_luminance"; svg = "examples\xge_svg\assets\mask_type_lowercase_luminance.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "mask_type_invalid_luminance"; svg = "examples\xge_svg\assets\mask_type_invalid_luminance.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "blend_inherit_normal"; svg = "examples\xge_svg\assets\blend_inherit_normal.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "filter_region_default_em"; svg = "examples\xge_svg\assets\filter_region_default_em.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "filter_primitive_default_em"; svg = "examples\xge_svg\assets\filter_primitive_default_em.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "symbol_default_em"; svg = "examples\xge_svg\assets\symbol_default_em.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "pattern_default_em"; svg = "examples\xge_svg\assets\pattern_default_em.svg"; tags = @("source-semantics-batch-15"); experimental = $true }
		$cases += [ordered]@{ name = "pattern_group_isolation"; svg = "examples\xge_svg\assets\pattern_group_isolation.svg"; tags = @("svg-pattern-group-batch-58", "pattern", "group", "opacity"); experimental = $true }
		$cases += [ordered]@{ name = "css_multi_class_order"; svg = "examples\xge_svg\assets\css_multi_class_order.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_dashoffset_not_copied"; svg = "examples\xge_svg\assets\css_dashoffset_not_copied.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_miterlimit_not_copied"; svg = "examples\xge_svg\assets\css_miterlimit_not_copied.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_filter_not_copied"; svg = "examples\xge_svg\assets\css_filter_not_copied.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_dasharray_none_not_copied"; svg = "examples\xge_svg\assets\css_dasharray_none_not_copied.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_duplicate_opacity_first"; svg = "examples\xge_svg\assets\css_duplicate_opacity_first.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_duplicate_display_first"; svg = "examples\xge_svg\assets\css_duplicate_display_first.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_duplicate_blend_first"; svg = "examples\xge_svg\assets\css_duplicate_blend_first.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_duplicate_paint_order_first"; svg = "examples\xge_svg\assets\css_duplicate_paint_order_first.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_duplicate_fill_last"; svg = "examples\xge_svg\assets\css_duplicate_fill_last.svg"; tags = @("css-source-batch-16"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_fill_value_last"; svg = "examples\xge_svg\assets\css_important_fill_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_stroke_value_last"; svg = "examples\xge_svg\assets\css_important_stroke_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_fill_opacity_value_last"; svg = "examples\xge_svg\assets\css_important_fill_opacity_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_stroke_opacity_value_last"; svg = "examples\xge_svg\assets\css_important_stroke_opacity_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_stroke_width_value_last"; svg = "examples\xge_svg\assets\css_important_stroke_width_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_dasharray_value_last"; svg = "examples\xge_svg\assets\css_important_dasharray_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_linecap_value_last"; svg = "examples\xge_svg\assets\css_important_linecap_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_linejoin_value_last"; svg = "examples\xge_svg\assets\css_important_linejoin_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_transform_value_last"; svg = "examples\xge_svg\assets\css_important_transform_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_color_value_last"; svg = "examples\xge_svg\assets\css_important_color_value_last.svg"; tags = @("css-important-merge-batch-17"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_opacity_first_value"; svg = "examples\xge_svg\assets\css_important_opacity_first_value.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_display_first_value"; svg = "examples\xge_svg\assets\css_important_display_first_value.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_paint_order_first_value"; svg = "examples\xge_svg\assets\css_important_paint_order_first_value.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_blend_first_value"; svg = "examples\xge_svg\assets\css_important_blend_first_value.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_important_text_anchor_first_value"; svg = "examples\xge_svg\assets\css_important_text_anchor_first_value.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_inline_important_fill_blocks_normal"; svg = "examples\xge_svg\assets\css_inline_important_fill_blocks_normal.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_inline_later_important_fill_wins"; svg = "examples\xge_svg\assets\css_inline_later_important_fill_wins.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_inline_important_opacity_blocks_normal"; svg = "examples\xge_svg\assets\css_inline_important_opacity_blocks_normal.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_inline_important_transform_blocks_normal"; svg = "examples\xge_svg\assets\css_inline_important_transform_blocks_normal.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_inline_important_dasharray_blocks_normal"; svg = "examples\xge_svg\assets\css_inline_important_dasharray_blocks_normal.svg"; tags = @("css-important-precedence-batch-18"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_class_before_attr"; svg = "examples\xge_svg\assets\css_clip_class_before_attr.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_attr_before_class"; svg = "examples\xge_svg\assets\css_clip_attr_before_class.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_class_before_inline"; svg = "examples\xge_svg\assets\css_clip_class_before_inline.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_inline_before_class"; svg = "examples\xge_svg\assets\css_clip_inline_before_class.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_class_before_attr"; svg = "examples\xge_svg\assets\css_mask_class_before_attr.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_attr_before_class"; svg = "examples\xge_svg\assets\css_mask_attr_before_class.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_class_before_inline"; svg = "examples\xge_svg\assets\css_mask_class_before_inline.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_inline_before_class"; svg = "examples\xge_svg\assets\css_mask_inline_before_class.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_tag_class_priority"; svg = "examples\xge_svg\assets\css_clip_tag_class_priority.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_multi_class_order"; svg = "examples\xge_svg\assets\css_mask_multi_class_order.svg"; tags = @("css-url-order-batch-19"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_class_url_then_none"; svg = "examples\xge_svg\assets\css_clip_class_url_then_none.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_class_url_then_invalid"; svg = "examples\xge_svg\assets\css_clip_class_url_then_invalid.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_class_url_then_none"; svg = "examples\xge_svg\assets\css_mask_class_url_then_none.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_class_url_then_invalid"; svg = "examples\xge_svg\assets\css_mask_class_url_then_invalid.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_inline_url_then_none"; svg = "examples\xge_svg\assets\css_clip_inline_url_then_none.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_inline_url_then_invalid"; svg = "examples\xge_svg\assets\css_clip_inline_url_then_invalid.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_inline_url_then_none"; svg = "examples\xge_svg\assets\css_mask_inline_url_then_none.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_inline_url_then_invalid"; svg = "examples\xge_svg\assets\css_mask_inline_url_then_invalid.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_clip_class_url_then_important_none"; svg = "examples\xge_svg\assets\css_clip_class_url_then_important_none.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_mask_inline_url_then_important_none"; svg = "examples\xge_svg\assets\css_mask_inline_url_then_important_none.svg"; tags = @("css-url-mutation-batch-20"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_class_before_clip_attr"; svg = "examples\xge_svg\assets\css_late_class_before_clip_attr.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_class_before_mask_attr"; svg = "examples\xge_svg\assets\css_late_class_before_mask_attr.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_class_before_clip_inline"; svg = "examples\xge_svg\assets\css_late_class_before_clip_inline.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_class_before_mask_inline"; svg = "examples\xge_svg\assets\css_late_class_before_mask_inline.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_clip_attr_before_class"; svg = "examples\xge_svg\assets\css_late_clip_attr_before_class.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_mask_inline_before_class"; svg = "examples\xge_svg\assets\css_late_mask_inline_before_class.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_class_before_fill_attr"; svg = "examples\xge_svg\assets\css_late_class_before_fill_attr.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_class_before_transform_attr"; svg = "examples\xge_svg\assets\css_late_class_before_transform_attr.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_multi_class_clip_order"; svg = "examples\xge_svg\assets\css_late_multi_class_clip_order.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "css_late_tag_class_clip_priority"; svg = "examples\xge_svg\assets\css_late_tag_class_clip_priority.svg"; tags = @("css-late-style-batch-21"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_clip_urlx"; svg = "examples\xge_svg\assets\url_prefix_clip_urlx.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_mask_urlx"; svg = "examples\xge_svg\assets\url_prefix_mask_urlx.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_clip_space"; svg = "examples\xge_svg\assets\url_prefix_clip_space.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_mask_space"; svg = "examples\xge_svg\assets\url_prefix_mask_space.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_inline_clip_replace"; svg = "examples\xge_svg\assets\url_prefix_inline_clip_replace.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_inline_mask_replace"; svg = "examples\xge_svg\assets\url_prefix_inline_mask_replace.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_class_clip_quoted"; svg = "examples\xge_svg\assets\url_prefix_class_clip_quoted.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_class_mask_quoted"; svg = "examples\xge_svg\assets\url_prefix_class_mask_quoted.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_duplicate_clip_malformed"; svg = "examples\xge_svg\assets\url_prefix_duplicate_clip_malformed.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "url_prefix_duplicate_mask_uppercase"; svg = "examples\xge_svg\assets\url_prefix_duplicate_mask_uppercase.svg"; tags = @("url-prefix-batch-22"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_fill_urlx"; svg = "examples\xge_svg\assets\paint_url_prefix_fill_urlx.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_stroke_urlx"; svg = "examples\xge_svg\assets\paint_url_prefix_stroke_urlx.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_fill_space"; svg = "examples\xge_svg\assets\paint_url_prefix_fill_space.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_stroke_space"; svg = "examples\xge_svg\assets\paint_url_prefix_stroke_space.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_inline_fill_quoted"; svg = "examples\xge_svg\assets\paint_url_prefix_inline_fill_quoted.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_inline_stroke_quoted"; svg = "examples\xge_svg\assets\paint_url_prefix_inline_stroke_quoted.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_class_fill_quoted"; svg = "examples\xge_svg\assets\paint_url_prefix_class_fill_quoted.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_class_stroke_quoted"; svg = "examples\xge_svg\assets\paint_url_prefix_class_stroke_quoted.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_duplicate_fill_malformed"; svg = "examples\xge_svg\assets\paint_url_prefix_duplicate_fill_malformed.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "paint_url_prefix_duplicate_stroke_uppercase"; svg = "examples\xge_svg\assets\paint_url_prefix_duplicate_stroke_uppercase.svg"; tags = @("paint-url-prefix-batch-23"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_shape_urlx"; svg = "examples\xge_svg\assets\filter_url_prefix_shape_urlx.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_group_urlx"; svg = "examples\xge_svg\assets\filter_url_prefix_group_urlx.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_shape_space"; svg = "examples\xge_svg\assets\filter_url_prefix_shape_space.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_shape_quoted"; svg = "examples\xge_svg\assets\filter_url_prefix_shape_quoted.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_inline_urlx"; svg = "examples\xge_svg\assets\filter_url_prefix_inline_urlx.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_inline_quoted"; svg = "examples\xge_svg\assets\filter_url_prefix_inline_quoted.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_duplicate_attr_malformed"; svg = "examples\xge_svg\assets\filter_url_prefix_duplicate_attr_malformed.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_duplicate_inline_malformed"; svg = "examples\xge_svg\assets\filter_url_prefix_duplicate_inline_malformed.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_duplicate_attr_uppercase"; svg = "examples\xge_svg\assets\filter_url_prefix_duplicate_attr_uppercase.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_url_prefix_class_ignored"; svg = "examples\xge_svg\assets\filter_url_prefix_class_ignored.svg"; tags = @("filter-url-prefix-batch-24"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_duplicate_id"; svg = "examples\xge_svg\assets\filter_source_duplicate_id.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_duplicate_region"; svg = "examples\xge_svg\assets\filter_source_duplicate_region.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_user_default_region"; svg = "examples\xge_svg\assets\filter_source_user_default_region.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_units_monotonic"; svg = "examples\xge_svg\assets\filter_source_units_monotonic.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_primitive_units_monotonic"; svg = "examples\xge_svg\assets\filter_source_primitive_units_monotonic.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_primitive_region_duplicate"; svg = "examples\xge_svg\assets\filter_source_primitive_region_duplicate.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_std_duplicate"; svg = "examples\xge_svg\assets\filter_source_std_duplicate.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_std_negative_reset"; svg = "examples\xge_svg\assets\filter_source_std_negative_reset.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_std_double_comma"; svg = "examples\xge_svg\assets\filter_source_std_double_comma.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "filter_source_std_em"; svg = "examples\xge_svg\assets\filter_source_std_em.svg"; tags = @("filter-source-batch-25"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_fill_rgb_replay"; svg = "examples\xge_svg\assets\css_color_fill_rgb_replay.svg"; tags = @("color-source-batch-27"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_fill_hsl_replay"; svg = "examples\xge_svg\assets\css_color_fill_hsl_replay.svg"; tags = @("color-source-batch-27"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_fill_hex_replay"; svg = "examples\xge_svg\assets\css_color_fill_hex_replay.svg"; tags = @("color-source-batch-27"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_fill_none_reactivate"; svg = "examples\xge_svg\assets\css_color_fill_none_reactivate.svg"; tags = @("color-source-batch-27"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_fill_none_final"; svg = "examples\xge_svg\assets\css_color_fill_none_final.svg"; tags = @("color-source-batch-27"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_stroke_rgb_replay"; svg = "examples\xge_svg\assets\css_color_stroke_rgb_replay.svg"; tags = @("color-source-batch-27"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_stroke_none_reactivate"; svg = "examples\xge_svg\assets\css_color_stroke_none_reactivate.svg"; tags = @("color-source-batch-27"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_current_replay"; svg = "examples\xge_svg\assets\css_color_current_replay.svg"; tags = @("color-source-batch-27"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_important_lock"; svg = "examples\xge_svg\assets\css_color_important_lock.svg"; tags = @("color-source-batch-27"); experimental = $true }
		$cases += [ordered]@{ name = "css_color_important_replay"; svg = "examples\xge_svg\assets\css_color_important_replay.svg"; tags = @("color-source-batch-27"); experimental = $true }
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
