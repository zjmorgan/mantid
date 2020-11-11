# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.
import unittest

from qtpy.QtWidgets import QApplication

from mantid.simpleapi import CreateSampleWorkspace, LoadInstrument
from mantidqt.utils.qt.testing import start_qapplication
from mantidqt.utils.qt.testing.qt_widget_finder import QtWidgetFinder
from mantidqt.widgets.instrumentview.presenter import InstrumentViewPresenter


@start_qapplication
class InstrumentViewTest(unittest.TestCase, QtWidgetFinder):
    def test_window_deleted_correctly(self):
        ws = CreateSampleWorkspace()
        LoadInstrument(ws, InstrumentName='MARI', RewriteSpectraMap=False)

        p = InstrumentViewPresenter(ws)
        self.assert_widget_created()

        p.close(ws.name())

        QApplication.processEvents()
        self.assertEqual(None, p.ads_observer)
        self.assert_widget_not_present("instr")
        self.assert_no_toplevel_widgets()

    def test_window_force_deleted_correctly(self):
        ws = CreateSampleWorkspace()
        LoadInstrument(ws, InstrumentName='MARI', RewriteSpectraMap=False)

        p = InstrumentViewPresenter(ws)
        self.assert_widget_created()

        p.force_close()

        QApplication.processEvents()
        self.assertEqual(None, p.ads_observer)
        self.assert_widget_not_present("instr")
        self.assert_no_toplevel_widgets()

    def test_select_and_get_tab(self):
        """Test launch and close instrument view with ARCS data
        """
        # create workspace
        ws = CreateSampleWorkspace()
        LoadInstrument(ws, InstrumentName='ARCS', RewriteSpectraMap=False)

        # No Qt widgets so far
        self.assert_no_toplevel_widgets()

        # create instrument view presenter
        iv_presenter = InstrumentViewPresenter(ws, parent=None, ads_observer=None)
        self.assert_widget_created()

        # select pick tab
        iv_presenter.select_pick_tab()
        # current_tab_index = iv_presenter.container.widget.getCurrentTab()
        # assert current_tab_index == 1

        # pick_tab = iv_presenter.get_pick_tab()
        # assert pick_tab

        # render tab
        # iv_presenter.select_render_tab()
        # current_tab_index = iv_presenter.container.widget.getCurrentTab()
        # assert current_tab_index == 0

        # render_tab = iv_presenter.get_render_tab()
        # assert render_tab

        # close
        iv_presenter.close(ws.name())
        # process events to close all the widgets
        QApplication.processEvents()
        # asset no more widgets
        self.assert_no_toplevel_widgets()
