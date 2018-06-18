from __future__ import (absolute_import, division, print_function)

import unittest
import sys
import mantid

from sans.gui_logic.models.create_state import (create_states, create_gui_state_from_userfile)
from sans.common.enums import (SANSInstrument, ISISReductionMode, SANSFacility)
from sans.gui_logic.models.state_gui_model import StateGuiModel
from sans.gui_logic.models.table_model import TableModel, TableIndexModel
from sans.gui_logic.presenter.gui_state_director import (GuiStateDirector)

if sys.version_info.major == 3:
    from unittest import mock
else:
    import mock

class GuiCommonTest(unittest.TestCase):

    def setUp(self):
        self.table_model = TableModel()
        self.state_gui_model = StateGuiModel({})
        table_index_model_0 = TableIndexModel(0, 'LOQ74044', '', '', '', '', '', '', '', '', '', '', '')
        table_index_model_1 = TableIndexModel(1, 'LOQ74044', '', '', '', '', '', '', '', '', '', '', '')
        self.table_model.add_table_entry(0, table_index_model_0)
        self.table_model.add_table_entry(1, table_index_model_1)


    @mock.patch('sans.gui_logic.models.create_state.__create_row_state')
    def test_create_states_returns_correct_number_of_states(self, create_row_state_mock):

        states = create_states(self.state_gui_model, self.table_model, SANSInstrument.LOQ, SANSFacility.ISIS)

        self.assertEqual(len(states), 2)

    @mock.patch('sans.gui_logic.models.create_state.__create_row_state')
    def test_create_states_returns_correct_number_of_states_for_specified_row_index(self, create_row_state_mock):

        states = create_states(self.state_gui_model, self.table_model, SANSInstrument.LOQ, SANSFacility.ISIS, row_index=1)

        self.assertEqual(len(states), 1)

    @mock.patch('sans.gui_logic.models.create_state.__create_row_state')
    def test_skips_empty_rows(self, create_row_state_mock):
        table_index_model = TableIndexModel(1, '', '', '', '', '', '', '', '', '', '', '', '')
        self.table_model.add_table_entry(1, table_index_model)

        states = create_states(self.state_gui_model, self.table_model, SANSInstrument.LOQ, SANSFacility.ISIS)

        self.assertEqual(len(states), 1)

    @mock.patch('sans.gui_logic.models.create_state.__create_row_state')
    def test_create_state_from_user_file_if_specified(self, create_row_state_mock):
        table_index_model = TableIndexModel(0, 'LOQ74044', '', '', '', '', '', '', '', '', '', '', '',
                                              user_file='MaskFile.txt')
        table_model = TableModel()
        table_model.add_table_entry(0, table_index_model)
        row_state_model = create_gui_state_from_userfile('MaskFile.txt')
        row_gui_state_director = GuiStateDirector(table_model, row_state_model, SANSFacility.ISIS)

        states = create_states(self.state_gui_model, table_model, SANSInstrument.LOQ, SANSFacility.ISIS)

        self.assertEqual(len(states), 1)
        create_row_state_mock.assert_called_once_with(row_gui_state_director, 0, SANSInstrument.LOQ, file_lookup=True)


if __name__ == '__main__':
    unittest.main()