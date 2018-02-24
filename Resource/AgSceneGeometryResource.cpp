#include "AgSceneGeometryResource.h"

namespace ambergris {

	int AgSceneGeometryResource::AppendInstance(int mesh, int node)
	{
		int i = 0;
		for each (auto itm in m_inst_pool)
		{
			if (itm.m_mesh == mesh)
			{
				itm.m_instances.push_back(node);
				return i;
			}
			i++;
		}

		GeometryInst* geom_inst = new GeometryInst;
		geom_inst->m_mesh = mesh;
		geom_inst->m_instances.push_back(node);
		m_inst_pool.push_back(geom_inst);
		return (int)m_inst_pool.size() - 1;
	}

	size_t AgSceneGeometryResource::GetInstanceSize(int id) const
	{
		if (id >= 0 && id < m_inst_pool.size())
			return m_inst_pool.at(id).m_instances.size();
		return 0;
	}

	int AgSceneGeometryResource::GetInstance(int id, int inst) const
	{
		if (id >= 0 && id < m_inst_pool.size())
		{
			const GeometryInst::INSTANCES& instances =
				m_inst_pool.at(id).m_instances;
			if (inst >= 0 && inst < instances.size())
				return instances.at(inst);
		}
		return -1;
	}
}