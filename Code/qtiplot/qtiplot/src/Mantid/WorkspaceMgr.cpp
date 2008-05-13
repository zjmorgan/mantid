#include <vector>
#include <string>
#include <QMessageBox>
#include <QListWidgetItem>

#include "WorkspaceMgr.h"
#include "../ApplicationWindow.h"
#include "../Matrix.h"
#include "LoadRawDlg.h"
#include "ImportWorkspaceDlg.h"
#include "ExecuteAlgorithm.h"

WorkspaceMgr::WorkspaceMgr(QWidget *parent) : QDialog(parent)
{
	m_parent = parent;
	
	setupUi(this);
	setupActions();
	
	m_interface = new Mantid::PythonAPI::PythonInterface;
	m_interface->InitialiseFrameworkManager();
	
	getWorkspaces();
	
	getAlgorithms();	
}

WorkspaceMgr::~WorkspaceMgr()
{
	
}

void WorkspaceMgr::setupActions()
{
	connect(pushExit, SIGNAL(clicked()), this, SLOT(close()));
	connect(pushAddWorkspace, SIGNAL(clicked()), this, SLOT(addWorkspaceClicked()));
	connect(removeWorkspaceButton, SIGNAL(clicked()), this, SLOT(deleteWorkspaceClicked()));
	connect(listWorkspaces, SIGNAL(itemSelectionChanged()), this, SLOT(selectedWorkspaceChanged()));
	connect(pushImportWorkspace, SIGNAL(clicked()), this, SLOT(importWorkspace()));
	connect(pushExecuteAlg, SIGNAL(clicked()), this, SLOT(executeAlgorithm()));
}

void WorkspaceMgr::getWorkspaces()
{
	listWorkspaces->clear();
	
	std::vector<std::string> names = m_interface->GetWorkspaceNames();

	for(unsigned int i = 0; i < names.size(); ++i) 
	{
		listWorkspaces->insertItem(0, QString::fromStdString(names[i]));
	}
}

void WorkspaceMgr::getAlgorithms()
{
	listAlgorithms->clear();
	
	std::vector<std::string> algs = m_interface->GetAlgorithmNames();
	
	for(unsigned int i = 0; i < algs.size(); ++i) 
	{
		listAlgorithms->insertItem(0, QString::fromStdString(algs[i]));
	}
}

void WorkspaceMgr::addWorkspaceClicked()
{
	loadRawDlg* dlg = new loadRawDlg(this);
	dlg->setModal(true);	
	dlg->exec();
	
	if (!dlg->getFilename().isEmpty())
	{	
		
		Mantid::API::Workspace_sptr ws = m_interface->LoadIsisRawFile(dlg->getFilename().toStdString(), dlg->getWorkspaceName().toStdString());
		if (ws.use_count() == 0)
		{
			QMessageBox::warning(this, tr("Mantid"),
                   		tr("A workspace with this name already exists.\n")
                    		, QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		
		getWorkspaces();

	}
}

void WorkspaceMgr::deleteWorkspaceClicked()
{
	if (listWorkspaces->currentRow() != -1)
	{
		QListWidgetItem *selected = listWorkspaces->item(listWorkspaces->currentRow());
		QString wsName = selected->text();
		
		m_interface->DeleteWorkspace(wsName.toStdString());
		
		listWorkspaces->setCurrentRow(-1);
		
		getWorkspaces();
	}
}


void WorkspaceMgr::selectedWorkspaceChanged()
{
	if (listWorkspaces->currentRow() != -1)
	{
		QListWidgetItem *selected = listWorkspaces->item(listWorkspaces->currentRow());
		QString wsName = selected->text();
		
		Mantid::API::Workspace_sptr ws = m_interface->RetrieveWorkspace(wsName.toStdString());
		
		int numHists = ws->getHistogramNumber();
		int numBins = ws->blocksize();
		
		textWorkspaceInfo->setPlainText("Number of histograms: " + QString::number(numHists) + "\nNumber of bins: " + QString::number(numBins));
	}
}

void WorkspaceMgr::importWorkspace()
{
	if (listWorkspaces->currentRow() != -1) //&& listWorkspaces->currentRow() != -1)
	{
		QListWidgetItem *selected = listWorkspaces->item(listWorkspaces->currentRow());
		QString wsName = selected->text();
		
		Mantid::API::Workspace_sptr ws = m_interface->RetrieveWorkspace(wsName.toStdString());
		
		int numHists = ws->getHistogramNumber();
		int numBins = ws->blocksize();
		
		ImportWorkspaceDlg* dlg = new ImportWorkspaceDlg(this, numHists);
		dlg->setModal(true);	
		if (dlg->exec() == QDialog::Accepted)
		{
			int start = dlg->getLowerLimit();
			int end = dlg->getUpperLimit();
			int diff = end - start;
			
			ApplicationWindow* temp = dynamic_cast<ApplicationWindow*>(m_parent);
			Matrix *mat = temp->newMatrix(wsName, numBins, end-start +1);

			int histCount = 0;
			
			for (int row = 0; row < numHists; ++row)
			{
				if (row >= start && row <= end)
				{
					std::vector<double>* Y = m_interface->GetYData(wsName.toStdString(), row);
					for (int col = 0; col < numBins; ++col)
					{
						mat->setCell(col, histCount, Y->at(col));
					}
					++histCount;
				}
			}	
		}
	}
}

void WorkspaceMgr:: executeAlgorithm()
{
	if (listAlgorithms->currentRow() != -1)
	{
		QListWidgetItem *selected = listAlgorithms->item(listAlgorithms->currentRow());
		QString algName = (selected->text()).split("|")[0];
		
		std::vector<std::string> propList = m_interface->GetAlgorithmProperties(algName.toStdString());

		if (propList.size() > 0)
		{
			ExecuteAlgorithm* dlg = new ExecuteAlgorithm(this);
			dlg->CreateLayout(propList);
			dlg->setModal(true);
		
			if (dlg->exec()== QDialog::Accepted)
			{
				Mantid::API::IAlgorithm* alg = m_interface->CreateAlgorithm(algName.toStdString());
				
				for (int i = 0; i < dlg->results.size(); ++i)
				{
					alg->setPropertyValue(propList[i], dlg->results[i]);
				}
				
				if (!alg->execute() == true)
				{
					//Algorithm did not execute properly
					int ret = QMessageBox::warning(this, tr("Mantid Algorithm"),
						tr("The algorithm failed to execute correctly. "
						"Please see the Mantid log for details."),
						QMessageBox::Ok);
				}
				
				getWorkspaces();
			}
		}
	}
}

