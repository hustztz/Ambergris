#include "AgSceneDatabase.h"

namespace ambergris {

	/*virtual*/
	/*AgResource::Handle AgSceneDatabase::Append(AgNode* node)
	{
		if (!node)
			return;

		


		int nNodeNum = (int)m_node_arr.size();
		for (int i = 0; i < nNodeNum; ++i)
		{
			if (m_node_arr[i].m_mesh_handle == node->m_mesh_handle)
			{
				node->m_inst_handle = Singleton<RenderResourceManager>::instance().mesh_manager.AppendInstance(node->m_mesh_handle, nNodeNum);
				m_node_arr[i].m_inst_handle = node->m_inst_handle;
				break;
			}
		}
		
		return AgResourceManager<AgNode>::Append(node);
	}*/
}