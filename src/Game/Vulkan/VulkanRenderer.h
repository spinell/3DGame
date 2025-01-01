#pragma once


class VulkanRenderer  {
public:
	VulkanRenderer() = default;
	~VulkanRenderer() = default;

	VulkanRenderer(const VulkanRenderer&) = delete;
	VulkanRenderer(VulkanRenderer&&) = delete;

	VulkanRenderer& operator=(const VulkanRenderer&) = delete;
	VulkanRenderer& operator=(VulkanRenderer&&) = delete;

	bool initialize();
private:
};