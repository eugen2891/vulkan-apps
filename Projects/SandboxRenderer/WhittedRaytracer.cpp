#include "WhittedRaytracer.hpp"

#include <VulkanKit/Resources.hpp>
#include <Sandbox/Geometry.hpp>

void WhittedRaytracer::buildAccelerationStructures(VkCommandBuffer commandBuffer)
{
	VkDeviceSize scratchMemSize = 0;
	VkAccelerationStructureGeometryKHR asg[sandbox::eMesh_EnumMax]{};
	VkAccelerationStructureBuildRangeInfoKHR asbri[sandbox::eMesh_EnumMax]{};
	VkAccelerationStructureBuildGeometryInfoKHR asbgi[sandbox::eMesh_EnumMax]{};
	VkAccelerationStructureBuildSizesInfoKHR asbsi[sandbox::eMesh_EnumMax]{};
	for (auto i = 0; i < sandbox::eMesh_EnumMax; i++)
	{
		VkAccelerationStructureGeometryKHR& mesh = asg[i];
		VkAccelerationStructureBuildRangeInfoKHR& range = asbri[i];
		VkAccelerationStructureBuildGeometryInfoKHR& info = asbgi[i];
		VkAccelerationStructureBuildSizesInfoKHR& size = asbsi[i];
		const sandbox::Drawable& drawable = sandbox::GetMesh(i);
		mesh.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		mesh.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		mesh.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		mesh.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		mesh.geometry.triangles.vertexData.deviceAddress = m_vbAddress;
		mesh.geometry.triangles.indexData.deviceAddress = m_ibAddress;
		mesh.geometry.triangles.vertexFormat = m_vertexFormat;
		mesh.geometry.triangles.vertexStride = m_vertexStride;
		mesh.geometry.triangles.indexType = m_indexType;
		mesh.geometry.triangles.maxVertex = drawable.vertexCount - 1;
		range.primitiveCount = drawable.indexCount / 3;
		range.primitiveOffset = uint32_t(drawable.firstIndex * m_indexStride);
		range.firstVertex = uint32_t(drawable.vertexOffset);
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
		info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		info.geometryCount = 1;
		info.pGeometries = &asg[i];
		size.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(m_vk.device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &info, &range.primitiveCount, &size);
		scratchMemSize = util::Max(scratchMemSize, size.buildScratchSize);
		//create a buffer
	}
	BreakIfNot(0);
	//const auto* rangesRef = asbri;
	//vkCmdBuildAccelerationStructuresKHR(commandBuffer, sandbox::eMesh_EnumMax, asbgi, &rangesRef);
}

void WhittedRaytracer::setVertexData(vulkan::Resource* buffer, VkFormat format, VkDeviceSize stride) noexcept
{
	m_vbAddress = buffer->bufferAddress();
	m_vertexFormat = format;
	m_vertexStride = stride;
}

void WhittedRaytracer::setIndexData(vulkan::Resource* buffer, VkIndexType indexType, VkDeviceSize stride) noexcept
{
	m_ibAddress = buffer->bufferAddress();
	m_indexType = indexType;
	m_indexStride = stride;
}
