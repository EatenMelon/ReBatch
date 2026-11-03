#include <BindGroupLayout.h>

#include <vector>
#include <algorithm>
#include <stdexcept>

#include <Renderer2D.h>

#include "GraphicsContext.h"
#include "Helper.h"

void rebatch::BindGroupLayout::SetUniformCount(size_t count)
{
	m_UniformEntries.resize(count);
}

bool rebatch::BindGroupLayout::AddTextureEntry(int binding)
{
	if (IsLocked()) return false;

	if (binding == m_UniformBinding)
	{
		throw std::runtime_error("Cannot bind texture to a binding reserved for uniforms");
	}

	WGPUBindGroupLayoutEntry entry{};
	entry.binding = binding;
	entry.visibility = WGPUShaderStage_Fragment;
	entry.texture.sampleType = WGPUTextureSampleType_Float;
	entry.texture.viewDimension = WGPUTextureViewDimension_2D;

	m_Entries.emplace(binding, entry);

	return true;
}

bool rebatch::BindGroupLayout::AddSamplerEntry(int binding)
{
	if (IsLocked()) return false;

	if (binding == m_UniformBinding)
	{
		throw std::runtime_error("Cannot bind samplers to a binding reserved for uniforms");
	}

	WGPUBindGroupLayoutEntry entry{};
	entry.binding = binding;
	entry.visibility = WGPUShaderStage_Fragment;
	entry.sampler.type = WGPUSamplerBindingType_Filtering;

	m_Entries.emplace(binding, entry);

	return true;
}

bool rebatch::BindGroupLayout::AddFrameEntry(int binding)
{
	if (IsLocked()) return false;

	if (binding == m_UniformBinding)
	{
		throw std::runtime_error("Cannot bind framebuffers to a binding reserved for uniforms");
	}

	WGPUBindGroupLayoutEntry entry{};
	entry.binding = binding;
	entry.visibility = WGPUShaderStage_Fragment;
	entry.texture.sampleType = WGPUTextureSampleType_Float;
	entry.texture.viewDimension = WGPUTextureViewDimension_2D;

	m_FrameEntry = std::make_unique<WGPUBindGroupLayoutEntry>(entry);

	return true;
}

int rebatch::BindGroupLayout::GetFrameEntryBinding() const
{
	if (m_FrameEntry == nullptr) return -1;

	return m_FrameEntry->binding;
}

void rebatch::BindGroupLayout::ConfirmLayout(const Renderer2D& renderer)
{
	m_Context = renderer.GetContext();

	std::vector<WGPUBindGroupLayoutEntry> entries{};

	if (m_UniformEntries.size() > 0)
	{
		WGPUBindGroupLayoutEntry entry{};
		entry.binding = m_UniformBinding;
		entry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
		entry.buffer.type = WGPUBufferBindingType_Uniform;

		for (const auto& uniformEntry : m_UniformEntries)
		{
			//m_UniformBufferSize += uniformEntry.size;
			m_UniformBufferSize += helper::Align(uniformEntry.size, 16);
		}

		entry.buffer.minBindingSize = m_UniformBufferSize;
		entries.push_back(entry);
	}

	for (const auto& [binding, entry] : m_Entries)
	{
		entries.push_back(entry);
	}

	if (m_FrameEntry != nullptr)
	{
		entries.push_back(*m_FrameEntry.get());
	}

	std::sort
	(
		entries.begin(),
		entries.end(),
		[](const WGPUBindGroupLayoutEntry& a, const WGPUBindGroupLayoutEntry& b)
		{
			return a.binding < b.binding;
		}
	);

	WGPUBindGroupLayoutDescriptor desc{};
	desc.nextInChain = nullptr;
	desc.entryCount = entries.size();
	desc.entries = entries.data();

	m_BindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_Context->GetDevice(), &desc);
}

int rebatch::BindGroupLayout::GetUniformEntryBinding() const
{
	if (m_UniformEntries.size() <= 0) return -1;

	return m_UniformBinding;
}

size_t rebatch::BindGroupLayout::GetUniformCount() const
{
	return m_UniformEntries.size();
}

uint64_t rebatch::BindGroupLayout::GetUniformSize(size_t location) const
{
	if (location >= m_UniformEntries.size()) return 0;

	return m_UniformEntries[location].size;

}

uint64_t rebatch::BindGroupLayout::GetRequiredUniformBufferSize() const
{
	return m_UniformBufferSize;
}
