#!/usr/bin/env python3
"""Regression tests for the public-header comment ratchet."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from api_comment_coverage import documented
from comment_lint import gate_failures


class CommentGateTests(unittest.TestCase):
    def setUp(self):
        self.baseline = {
            "all_apis": {"xui.h": ["oldRemoved", "stable", "oldUndocumented"]},
            "documented_apis": {"xui.h": ["oldRemoved", "stable"]},
        }

    def test_removed_api_does_not_lower_gate(self):
        self.assertEqual(gate_failures("xui.h", [("stable", 1, True)], self.baseline), [])

    def test_existing_documented_api_losing_comment_fails(self):
        failures = gate_failures("xui.h", [("stable", 1, False)], self.baseline)
        self.assertEqual(failures, ["gate2 xui.h: documented API lost comment -> stable"])

    def test_new_undocumented_api_fails(self):
        failures = gate_failures("xui.h", [("added", 1, False)], self.baseline)
        self.assertEqual(failures, ["gate1 xui.h: new API without comment -> added"])

    def test_existing_undocumented_api_getting_comment_passes(self):
        self.assertEqual(
            gate_failures("xui.h", [("oldUndocumented", 1, True)], self.baseline), []
        )

    def test_new_header_is_checked(self):
        failures = gate_failures("xui_document.h", [("newApi", 1, False)], self.baseline)
        self.assertEqual(failures, ["gate1 xui_document.h: new API without comment -> newApi"])

    def test_feature_guard_does_not_hide_attached_comment(self):
        lines = [
            "/* Returns the widget type. */",
            "#if XUI_ENABLE_WIDGET",
            "XUI_API xui_widget_type xuiWidgetGetType(xui_context context);",
        ]
        self.assertTrue(documented(lines, 2))

    def test_nested_feature_guards_do_not_hide_attached_comment(self):
        lines = [
            "/* Returns the widget type. */",
            "#if XGE_ENABLE_XUI",
            "#if XUI_ENABLE_WIDGET",
            "XUI_API xui_widget_type xuiWidgetGetType(xui_context context);",
        ]
        self.assertTrue(documented(lines, 3))

    def test_nonconditional_preprocessor_line_breaks_comment_attachment(self):
        lines = [
            "/* This documents the macro, not the API below it. */",
            "#define XUI_WIDGET_LIMIT 8",
            "XUI_API xui_widget_type xuiWidgetGetType(xui_context context);",
        ]
        self.assertFalse(documented(lines, 2))

    def test_previous_declaration_trailing_comment_is_not_reused(self):
        lines = [
            "XUI_API int xuiFirst(void); /* Documents only xuiFirst. */",
            "XUI_API int xuiSecond(void);",
        ]
        self.assertFalse(documented(lines, 1))


if __name__ == "__main__":
    unittest.main()
