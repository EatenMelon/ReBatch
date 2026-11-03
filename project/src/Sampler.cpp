#include <Sampler.h>
#include <Renderer2D.h>
#include "GraphicsContext.h"

rebatch::Sampler::Sampler(const Renderer2D& renderer, Preset preset)
	: Sampler(renderer, GetSettings(preset))
{}

rebatch::Sampler::Sampler(const Renderer2D& renderer, AddressMode u, AddressMode v, FilterMode mag, FilterMode min)
	: m_Context{ renderer.GetContext()}
{
	WGPUSamplerDescriptor samplerDesc{};
	samplerDesc.addressModeU = GetWGPUAddressMode(u);
	samplerDesc.addressModeV = GetWGPUAddressMode(v);

	samplerDesc.addressModeW = WGPUAddressMode_ClampToEdge;

	samplerDesc.magFilter = GetFilterMode(mag);
	samplerDesc.minFilter = GetFilterMode(min);

	samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
	samplerDesc.lodMinClamp = 0.0f;
	samplerDesc.lodMaxClamp = 0.0f;

	samplerDesc.compare = WGPUCompareFunction_Undefined;
	samplerDesc.maxAnisotropy = 1;

	m_Sampler = wgpuDeviceCreateSampler(m_Context->GetDevice(), &samplerDesc);
}

rebatch::Sampler::~Sampler() noexcept
{
	wgpuSamplerRelease(m_Sampler);
}

rebatch::Sampler::Sampler(const Renderer2D& renderer, SamplerSettings settings)
	: Sampler(renderer, settings.addressModeU, settings.addressModeV, settings.magFilter, settings.minFilter)
{}

rebatch::Sampler::SamplerSettings rebatch::Sampler::GetSettings(Preset preset)
{
	SamplerSettings settings{};

	settings.addressModeU = AddressMode::Repeat;
	settings.addressModeV = AddressMode::Repeat;

	switch (preset)
	{
	case rebatch::Sampler::Preset::Nearest:
		settings.magFilter = FilterMode::Nearest;
		settings.minFilter = FilterMode::Nearest;
		break;

	default:
		settings.magFilter = FilterMode::Linear;
		settings.minFilter = FilterMode::Linear;
		break;
	}

	return settings;
}

WGPUAddressMode rebatch::Sampler::GetWGPUAddressMode(AddressMode mode)
{
	switch (mode)
	{
	case rebatch::AddressMode::MirrorRepeat:	return WGPUAddressMode_MirrorRepeat;
	case rebatch::AddressMode::Clamp:			return WGPUAddressMode_ClampToEdge;
	default:								return WGPUAddressMode_Repeat;
	}
}

WGPUFilterMode rebatch::Sampler::GetFilterMode(FilterMode mode)
{
	switch (mode)
	{
	case rebatch::FilterMode::Linear:	return WGPUFilterMode_Linear;
	default:						return WGPUFilterMode_Nearest;
	}
}
