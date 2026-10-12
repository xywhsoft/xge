#!/usr/bin/env python3
"""Regression tests for the public-header comment ratchet."""

import unittest

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


if __name__ == "__main__":
    unittest.main()
