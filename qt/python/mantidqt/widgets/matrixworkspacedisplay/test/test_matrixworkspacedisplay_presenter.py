# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.
#
#
from __future__ import (absolute_import, division, print_function)

import unittest

from mock import Mock, call

from mantidqt.widgets.matrixworkspacedisplay.presenter import MatrixWorkspaceDisplay
from mantidqt.widgets.matrixworkspacedisplay.test_helpers.matrixworkspacedisplay_common import MockWorkspace, \
    MockQModelIndex, MockQModelIndexSibling
from mantidqt.widgets.matrixworkspacedisplay.test_helpers.mock_matrixworkspacedisplay import \
    MockMatrixWorkspaceDisplayView, MockQTableView

from mantidqt.widgets.matrixworkspacedisplay.test_helpers.mockplotlib import MockPlotLib


class MatrixWorkspaceDisplayPresenterTest(unittest.TestCase):
    def assertNotCalled(self, mock):
        # TODO move this into a helper class, extend mock, or extend TestCase
        self.assertEqual(0, mock.call_count)

    def test_setup_table(self):
        ws = MockWorkspace()
        view = MockMatrixWorkspaceDisplayView()
        presenter = MatrixWorkspaceDisplay(ws, view=view)
        self.assertEqual(3, view.set_context_menu_actions.call_count)
        self.assertEqual(1, view.set_model.call_count)

    def test_action_copy_spectrum_values(self):
        ws = MockWorkspace()
        view = MockMatrixWorkspaceDisplayView()
        presenter = MatrixWorkspaceDisplay(ws, view=view)

        mock_table = MockQTableView()

        # two rows are selected in different positions
        mock_indexes = [MockQModelIndex(0, 1), MockQModelIndex(3, 1)]
        mock_table.mock_selection_model.selectedRows = Mock(return_value=mock_indexes)
        mock_read = Mock(return_value=[43, 99])
        expected_string = "43 99\n43 99"

        presenter.action_copy_spectrum_values(mock_table, mock_read)

        mock_table.selectionModel.assert_called_once_with()
        mock_table.mock_selection_model.hasSelection.assert_called_once_with()
        view.copy_to_clipboard.assert_called_once_with(expected_string)
        view.show_mouse_toast.assert_called_once_with(MatrixWorkspaceDisplay.COPY_SUCCESSFUL_MESSAGE)

    def test_action_copy_spectrum_values_no_selection(self):
        ws = MockWorkspace()
        view = MockMatrixWorkspaceDisplayView()
        presenter = MatrixWorkspaceDisplay(ws, view=view)

        mock_table = MockQTableView()
        mock_table.mock_selection_model.hasSelection = Mock(return_value=False)
        mock_table.mock_selection_model.selectedRows = Mock()

        presenter.action_copy_spectrum_values(mock_table, None)

        mock_table.selectionModel.assert_called_once_with()
        mock_table.mock_selection_model.hasSelection.assert_called_once_with()
        # the action should never look for rows if there is no selection
        self.assertNotCalled(mock_table.mock_selection_model.selectedRows)
        view.show_mouse_toast.assert_called_once_with(MatrixWorkspaceDisplay.NO_SELECTION_MESSAGE)

    def test_action_copy_bin_values(self):
        ws = MockWorkspace()
        view = MockMatrixWorkspaceDisplayView()
        presenter = MatrixWorkspaceDisplay(ws, view=view)
        mock_table = MockQTableView()

        # two columns are selected at different positions
        mock_indexes = [MockQModelIndex(0, 0), MockQModelIndex(0, 3)]
        mock_table.mock_selection_model.selectedColumns = Mock(return_value=mock_indexes)
        # change the mock ws to have 3 histograms
        ws.getNumberHistograms = Mock(return_value=3)

        mock_read = Mock(return_value=[83, 11, 33, 70])
        expected_string = "83 70\n83 70\n83 70"

        presenter.action_copy_bin_values(mock_table, mock_read)

        mock_table.selectionModel.assert_called_once_with()
        view.copy_to_clipboard.assert_called_once_with(expected_string)
        mock_table.mock_selection_model.hasSelection.assert_called_once_with()
        view.show_mouse_toast.assert_called_once_with(MatrixWorkspaceDisplay.COPY_SUCCESSFUL_MESSAGE)

    def test_action_copy_bin_values_no_selection(self):
        ws = MockWorkspace()
        view = MockMatrixWorkspaceDisplayView()
        presenter = MatrixWorkspaceDisplay(ws, view=view)

        mock_table = MockQTableView()
        mock_table.mock_selection_model.hasSelection = Mock(return_value=False)
        mock_table.mock_selection_model.selectedColumns = Mock()

        presenter.action_copy_bin_values(mock_table, None)

        mock_table.selectionModel.assert_called_once_with()
        mock_table.mock_selection_model.hasSelection.assert_called_once_with()
        # the action should never look for rows if there is no selection
        self.assertNotCalled(mock_table.mock_selection_model.selectedColumns)
        view.show_mouse_toast.assert_called_once_with(MatrixWorkspaceDisplay.NO_SELECTION_MESSAGE)

    def test_action_copy_cell(self):
        ws = MockWorkspace()
        view = MockMatrixWorkspaceDisplayView()
        presenter = MatrixWorkspaceDisplay(ws, view=view)
        mock_table = MockQTableView()

        # two columns are selected at different positions
        mock_table.mock_selection_model.currentIndex = Mock(return_value=MockQModelIndex(0, 2))
        # change the mock ws to have 3 histograms
        ws.getNumberHistograms = Mock(return_value=3)
        presenter.action_copy_cell(mock_table)

        mock_table.selectionModel.assert_called_once_with()
        view.copy_to_clipboard.assert_called_once_with(MockQModelIndexSibling.TEST_SIBLING_DATA)
        view.show_mouse_toast.assert_called_once_with(MatrixWorkspaceDisplay.COPY_SUCCESSFUL_MESSAGE)

    def test_action_copy_cell_no_selection(self):
        ws = MockWorkspace()
        view = MockMatrixWorkspaceDisplayView()
        presenter = MatrixWorkspaceDisplay(ws, view=view)
        mock_table = MockQTableView()

        # two columns are selected at different positions
        mock_table.mock_selection_model.currentIndex = Mock(return_value=MockQModelIndex(0, 2))
        # change the mock ws to have 3 histograms
        ws.getNumberHistograms = Mock(return_value=3)
        presenter.action_copy_cell(mock_table)

        mock_table.selectionModel.assert_called_once_with()
        view.copy_to_clipboard.assert_called_once_with(MockQModelIndexSibling.TEST_SIBLING_DATA)
        view.show_mouse_toast.assert_called_once_with(MatrixWorkspaceDisplay.COPY_SUCCESSFUL_MESSAGE)

    def test_action_plot_spectrum(self):
        mock_ws = MockWorkspace()
        mock_view = MockMatrixWorkspaceDisplayView()
        mock_plot = MockPlotLib()

        presenter = MatrixWorkspaceDisplay(mock_ws, plt=mock_plot, view=mock_view)

        mock_table = MockQTableView()

        # configure the mock return values
        mock_table.mock_selection_model.hasSelection = Mock(return_value=True)
        mock_selected_rows_return_value = [MockQModelIndex(1, 1), MockQModelIndex(2, 1)]
        mock_table.mock_selection_model.selectedRows = Mock(return_value=mock_selected_rows_return_value)
        mock_table.mock_selection_model.selectedColumns = Mock()
        mock_view.ask_confirmation = Mock()

        # monkey-patch the spectrum plot label to count the number of calls
        presenter.model.get_spectrum_plot_label = Mock()

        presenter.action_plot_spectrum(mock_table)

        mock_table.selectionModel.assert_called_once_with()
        mock_table.mock_selection_model.hasSelection.assert_called_once_with()
        mock_table.mock_selection_model.selectedRows.assert_called_once_with()
        self.assertNotCalled(mock_table.mock_selection_model.selectedColumns)
        # Confirmation should not have been asked for as the length is less than required
        self.assertNotCalled(mock_view.ask_confirmation)

        mock_plot.subplots.assert_called_once_with(subplot_kw={'projection': 'mantid'})
        presenter.model.get_spectrum_plot_label.assert_has_calls([call(1), call(2)])
        # plot must be called the same number of times as selected indices
        self.assertEqual(len(mock_selected_rows_return_value), mock_plot.mock_ax.plot.call_count)
        # errorbar is not being added here
        self.assertNotCalled(mock_plot.mock_ax.errorbar)

        mock_plot.mock_ax.legend.assert_called_once_with()
        mock_plot.mock_fig.show.assert_called_once_with()

    def test_action_plot_spectrum_selection_larger_than_10_confirmed(self):
        mock_ws = MockWorkspace()
        mock_view = MockMatrixWorkspaceDisplayView()
        mock_plot = MockPlotLib()

        presenter = MatrixWorkspaceDisplay(mock_ws, plt=mock_plot, view=mock_view)

        mock_table = MockQTableView()

        # configure the mock return values
        mock_table.mock_selection_model.hasSelection = Mock(return_value=True)
        mock_selected_rows_return_value = []
        num_selected_rows = MatrixWorkspaceDisplay.NUM_SELECTED_FOR_CONFIRMATION + 1

        for i in range(num_selected_rows):
            mock_selected_rows_return_value.append(MockQModelIndex(i, 1))

        mock_table.mock_selection_model.selectedRows = Mock(return_value=mock_selected_rows_return_value)
        mock_table.mock_selection_model.selectedColumns = Mock()

        # The a lot of things to plot message will show, set that the user will CONFIRM the plot
        # meaning the rest of the function will execute as normal
        mock_view.ask_confirmation = Mock(return_value=True)

        # monkey-patch the spectrum plot label to count the number of calls
        presenter.model.get_spectrum_plot_label = Mock()

        presenter.action_plot_spectrum(mock_table)

        mock_table.selectionModel.assert_called_once_with()
        mock_table.mock_selection_model.hasSelection.assert_called_once_with()
        mock_table.mock_selection_model.selectedRows.assert_called_once_with()

        mock_view.ask_confirmation.assert_called_once_with(
            MatrixWorkspaceDisplay.A_LOT_OF_THINGS_TO_PLOT_MESSAGE.format(num_selected_rows))

        self.assertNotCalled(mock_table.mock_selection_model.selectedColumns)

        mock_plot.subplots.assert_called_once_with(subplot_kw={'projection': 'mantid'})
        self.assertEqual(len(mock_selected_rows_return_value), presenter.model.get_spectrum_plot_label.call_count)
        # plot must be called the same number of times as selected indices
        self.assertEqual(len(mock_selected_rows_return_value), mock_plot.mock_ax.plot.call_count)
        # errorbar is not being added here
        self.assertNotCalled(mock_plot.mock_ax.errorbar)

        mock_plot.mock_ax.legend.assert_called_once_with()
        mock_plot.mock_fig.show.assert_called_once_with()

    def test_action_plot_spectrum_selection_larger_than_10_denied(self):
        mock_ws = MockWorkspace()
        mock_view = MockMatrixWorkspaceDisplayView()
        mock_plot = MockPlotLib()

        presenter = MatrixWorkspaceDisplay(mock_ws, plt=mock_plot, view=mock_view)

        mock_table = MockQTableView()

        # configure the mock return values
        mock_table.mock_selection_model.hasSelection = Mock(return_value=True)
        num_selected_rows = MatrixWorkspaceDisplay.NUM_SELECTED_FOR_CONFIRMATION + 1
        mock_selected_rows_return_value = []
        for i in range(num_selected_rows):
            mock_selected_rows_return_value.append(MockQModelIndex(i, 1))

        mock_table.mock_selection_model.selectedRows = Mock(return_value=mock_selected_rows_return_value)
        mock_table.mock_selection_model.selectedColumns = Mock()

        # The a lot of things to plot message will show, set that the user will DENY the plot
        # meaning the rest of the function will NOT EXECUTE AT ALL
        mock_view.ask_confirmation = Mock(return_value=False)

        # monkey-patch the spectrum plot label to count the number of calls
        presenter.model.get_spectrum_plot_label = Mock()

        presenter.action_plot_spectrum(mock_table)

        mock_table.selectionModel.assert_called_once_with()
        mock_table.mock_selection_model.hasSelection.assert_called_once_with()
        mock_table.mock_selection_model.selectedRows.assert_called_once_with()

        mock_view.ask_confirmation.assert_called_once_with(
            MatrixWorkspaceDisplay.A_LOT_OF_THINGS_TO_PLOT_MESSAGE.format(num_selected_rows))

        self.assertNotCalled(mock_table.mock_selection_model.selectedColumns)
        self.assertNotCalled(mock_plot.subplots)
        self.assertNotCalled(presenter.model.get_spectrum_plot_label)
        self.assertNotCalled(mock_plot.mock_ax.plot)
        self.assertNotCalled(mock_plot.mock_ax.errorbar)
        self.assertNotCalled(mock_plot.mock_ax.legend)
        self.assertNotCalled(mock_plot.mock_fig.show)

    def test_action_plot_spectrum_no_selection(self):
        mock_ws = MockWorkspace()
        mock_view = MockMatrixWorkspaceDisplayView()
        mock_plot = MockPlotLib()

        presenter = MatrixWorkspaceDisplay(mock_ws, plt=mock_plot, view=mock_view)

        mock_table = MockQTableView()

        # No selection means most of the function will not be executed
        mock_table.mock_selection_model.hasSelection = Mock(return_value=False)

        mock_table.mock_selection_model.selectedRows = Mock()
        mock_table.mock_selection_model.selectedColumns = Mock()
        presenter.model.get_spectrum_plot_label = Mock()

        presenter.action_plot_spectrum(mock_table)

        mock_view.show_mouse_toast.assert_called_once_with(MatrixWorkspaceDisplay.NO_SELECTION_MESSAGE)
        mock_table.selectionModel.assert_called_once_with()
        mock_table.mock_selection_model.hasSelection.assert_called_once_with()

        self.assertNotCalled(mock_table.mock_selection_model.selectedRows)
        self.assertNotCalled(mock_table.mock_selection_model.selectedColumns)
        self.assertNotCalled(mock_plot.subplots)
        self.assertNotCalled(presenter.model.get_spectrum_plot_label)
        self.assertNotCalled(mock_plot.mock_ax.plot)
        self.assertNotCalled(mock_plot.mock_ax.errorbar)
        self.assertNotCalled(mock_plot.mock_ax.legend)
        self.assertNotCalled(mock_plot.mock_fig.show)

    def test_action_plot_spectrum_with_errors(self):
        self.skipTest("Not implemented")

    def test_action_plot_bin(self):
        self.skipTest("Not implemented")

    def test_action_plot_bin_with_errors(self):
        self.skipTest("Not implemented")


if __name__ == '__main__':
    unittest.main()
