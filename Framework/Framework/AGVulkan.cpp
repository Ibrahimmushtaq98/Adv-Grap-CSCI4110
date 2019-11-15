/*
* Vulkan Samples
*
* Copyright (C) 2015-2016 Valve Corporation
* Copyright (C) 2015-2016 LunarG, Inc.
* Copyright (C) 2015-2016 Google, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/*
VULKAN_SAMPLE_DESCRIPTION
samples "init" utility functions
*/

#include <cstdlib>
#include <assert.h>
#include <string.h>
#include "AGVulkan.h"
#include "cube_data.h"

using namespace std;

VkResult init_global_extension_properties(layer_properties &layer_props) {
	VkExtensionProperties *instance_extensions;
	uint32_t instance_extension_count;
	VkResult res;
	char *layer_name = NULL;

	layer_name = layer_props.properties.layerName;

	do {
		res = vkEnumerateInstanceExtensionProperties(
			layer_name, &instance_extension_count, NULL);
		if (res)
			return res;

		if (instance_extension_count == 0) {
			return VK_SUCCESS;
		}

		layer_props.extensions.resize(instance_extension_count);
		instance_extensions = layer_props.extensions.data();
		res = vkEnumerateInstanceExtensionProperties(
			layer_name, &instance_extension_count, instance_extensions);
	} while (res == VK_INCOMPLETE);

	return res;
}

VkResult init_global_layer_properties(struct AGContext &info) {
	uint32_t instance_layer_count;
	VkLayerProperties *vk_props = NULL;
	VkResult res;


	/*
	* It's possible, though very rare, that the number of
	* instance layers could change. For example, installing something
	* could include new layers that the loader would pick up
	* between the initial query for the count and the
	* request for VkLayerProperties. The loader indicates that
	* by returning a VK_INCOMPLETE status and will update the
	* the count parameter.
	* The count parameter will be updated with the number of
	* entries loaded into the data pointer - in case the number
	* of layers went down or is smaller than the size given.
	*/
	do {
		res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
		if (res)
			return res;

		if (instance_layer_count == 0) {
			return VK_SUCCESS;
		}

		vk_props = (VkLayerProperties *)realloc(
			vk_props, instance_layer_count * sizeof(VkLayerProperties));

		res =
			vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
	} while (res == VK_INCOMPLETE);

	/*
	* Now gather the extension list for each instance layer.
	*/
	for (uint32_t i = 0; i < instance_layer_count; i++) {
		layer_properties layer_props;
		layer_props.properties = vk_props[i];
		res = init_global_extension_properties(layer_props);
		if (res)
			return res;
		info.instance_layer_properties.push_back(layer_props);
	}
	free(vk_props);

	return res;
}

VkResult init_device_extension_properties(struct AGContext &info,
	layer_properties &layer_props) {
	VkExtensionProperties *device_extensions;
	uint32_t device_extension_count;
	VkResult res;
	char *layer_name = NULL;

	layer_name = layer_props.properties.layerName;

	do {
		res = vkEnumerateDeviceExtensionProperties(
			info.gpus[0], layer_name, &device_extension_count, NULL);
		if (res)
			return res;

		if (device_extension_count == 0) {
			return VK_SUCCESS;
		}

		layer_props.extensions.resize(device_extension_count);
		device_extensions = layer_props.extensions.data();
		res = vkEnumerateDeviceExtensionProperties(info.gpus[0], layer_name,
			&device_extension_count,
			device_extensions);
	} while (res == VK_INCOMPLETE);

	return res;
}

VkBool32 demo_check_layers(const std::vector<layer_properties> &layer_props,
	const std::vector<const char *> &layer_names) {
	uint32_t check_count = layer_names.size();
	uint32_t layer_count = layer_props.size();
	for (uint32_t i = 0; i < check_count; i++) {
		VkBool32 found = 0;
		for (uint32_t j = 0; j < layer_count; j++) {
			if (!strcmp(layer_names[i], layer_props[j].properties.layerName)) {
				found = 1;
			}
		}
		if (!found) {
			std::cout << "Cannot find layer: " << layer_names[i] << std::endl;
			return 0;
		}
	}
	return 1;
}

void init_instance_extension_names(struct AGContext &info) {
	info.instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
	info.instance_extension_names.push_back(
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
	info.instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
}

VkResult init_instance(struct AGContext &info,
	char const *const app_short_name) {
#ifdef _WIN32
	strcpy(info.name,app_short_name);
#endif
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = app_short_name;
	app_info.applicationVersion = 1;
	app_info.pEngineName = app_short_name;
	app_info.engineVersion = 1;
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo inst_info = {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = NULL;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledLayerCount = info.instance_layer_names.size();
	inst_info.ppEnabledLayerNames = info.instance_layer_names.size()
		? info.instance_layer_names.data()
		: NULL;
	inst_info.enabledExtensionCount = info.instance_extension_names.size();
	inst_info.ppEnabledExtensionNames = info.instance_extension_names.data();

	VkResult res = vkCreateInstance(&inst_info, NULL, &info.inst);
	assert(res == VK_SUCCESS);

	return res;
}

void init_device_extension_names(struct AGContext &info) {
	info.device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VkResult init_device(struct AGContext &info) {
	VkResult res;
	VkDeviceQueueCreateInfo queue_info = {};

	float queue_priorities[1] = { 0.0 };
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = NULL;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = queue_priorities;
	queue_info.queueFamilyIndex = info.graphics_queue_family_index;

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = NULL;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = info.device_extension_names.size();
	device_info.ppEnabledExtensionNames =
		device_info.enabledExtensionCount ? info.device_extension_names.data()
		: NULL;
	device_info.pEnabledFeatures = NULL;

	res = vkCreateDevice(info.gpus[0], &device_info, NULL, &info.device);
	assert(res == VK_SUCCESS);

	return res;
}

VkResult init_enumerate_device(struct AGContext &info, uint32_t gpu_count) {
	uint32_t const U_ASSERT_ONLY req_count = gpu_count;
	VkResult res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, NULL);
	assert(gpu_count);
	info.gpus.resize(gpu_count);

	res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, info.gpus.data());
	assert(!res && gpu_count >= req_count);

	vkGetPhysicalDeviceQueueFamilyProperties(info.gpus[0],
		&info.queue_family_count, NULL);
	assert(info.queue_family_count >= 1);

	info.queue_props.resize(info.queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(
		info.gpus[0], &info.queue_family_count, info.queue_props.data());
	assert(info.queue_family_count >= 1);

	/* This is as good a place as any to do this */
	vkGetPhysicalDeviceMemoryProperties(info.gpus[0], &info.memory_properties);
	vkGetPhysicalDeviceProperties(info.gpus[0], &info.gpu_props);

	return res;
}

void init_queue_family_index(struct AGContext &info) {
	/* This routine simply finds a graphics queue for a later vkCreateDevice,
	* without consideration for which queue family can present an image.
	* Do not use this if your intent is to present later in your sample,
	* instead use the init_connection, init_window, init_swapchain_extension,
	* init_device call sequence to get a graphics and present compatible queue
	* family
	*/

	vkGetPhysicalDeviceQueueFamilyProperties(info.gpus[0],
		&info.queue_family_count, NULL);
	assert(info.queue_family_count >= 1);

	info.queue_props.resize(info.queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(
		info.gpus[0], &info.queue_family_count, info.queue_props.data());
	assert(info.queue_family_count >= 1);

	bool found = false;
	for (unsigned int i = 0; i < info.queue_family_count; i++) {
		if (info.queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			info.graphics_queue_family_index = i;
			found = true;
			break;
		}
	}
	assert(found);
}

void init_connection(struct AGContext &info) {
#if !defined(_WIN32)
	const xcb_setup_t *setup;
	xcb_screen_iterator_t iter;
	int scr;

	info.connection = xcb_connect(NULL, &scr);
	if (info.connection == NULL || xcb_connection_has_error(info.connection)) {
		std::cout << "Cannot find a compatible Vulkan ICD.\n";
		exit(-1);
	}

	setup = xcb_get_setup(info.connection);
	iter = xcb_setup_roots_iterator(setup);
	while (scr-- > 0)
		xcb_screen_next(&iter);

	info.screen = iter.data;
#endif //__Android__
}
#ifdef _WIN32
static void run(struct sample_info *info) {
	/* Placeholder for samples that want to show dynamic content */
}

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	struct sample_info *info = reinterpret_cast<struct sample_info *>(
		GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		run(info);
		return 0;
	default:
		break;
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void init_window(struct AGContext &info) {
	WNDCLASSEX win_class;
	assert(info.width > 0);
	assert(info.height > 0);

	info.connection = GetModuleHandle(NULL);

	// Initialize the window class structure:
	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WndProc;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = info.connection; // hInstance
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = NULL;
	wchar_t wname[256];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, info.name, -1, wname, 256);
	win_class.lpszClassName = (LPCWSTR) wname;
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	// Register window class:
	if (!RegisterClassEx(&win_class)) {
		// It didn't work, so try to give a useful error:
		printf("Unexpected error trying to start the application!\n");
		fflush(stdout);
		exit(1);
	}
	// Create window with the registered class:
	RECT wr = { 0, 0, info.width, info.height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	info.window = CreateWindowEx(0,
		(LPCWSTR) wname,            // class name
		(LPCWSTR) wname,            // app name
		WS_OVERLAPPEDWINDOW | // window style
		WS_VISIBLE | WS_SYSMENU,
		100, 100,           // x/y coords
		wr.right - wr.left, // width
		wr.bottom - wr.top, // height
		NULL,               // handle to parent
		NULL,               // handle to menu
		info.connection,    // hInstance
		NULL);              // no extra parameters
	if (!info.window) {
		// It didn't work, so try to give a useful error:
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}
	SetWindowLongPtr(info.window, GWLP_USERDATA, (LONG_PTR)&info);
}

void destroy_window(struct AGContext &info) {
	vkDestroySurfaceKHR(info.inst, info.surface, NULL);
	DestroyWindow(info.window);
}
#else
void init_window(struct AGContext &info) {
	assert(info.width > 0);
	assert(info.height > 0);

	uint32_t value_mask, value_list[32];

	info.window = xcb_generate_id(info.connection);

	value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	value_list[0] = info.screen->black_pixel;
	value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE;

	xcb_create_window(info.connection, XCB_COPY_FROM_PARENT, info.window,
		info.screen->root, 0, 0, info.width, info.height, 0,
		XCB_WINDOW_CLASS_INPUT_OUTPUT, info.screen->root_visual,
		value_mask, value_list);

	/* Magic code that will send notification when window is destroyed */
	xcb_intern_atom_cookie_t cookie =
		xcb_intern_atom(info.connection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t *reply =
		xcb_intern_atom_reply(info.connection, cookie, 0);

	xcb_intern_atom_cookie_t cookie2 =
		xcb_intern_atom(info.connection, 0, 16, "WM_DELETE_WINDOW");
	info.atom_wm_delete_window =
		xcb_intern_atom_reply(info.connection, cookie2, 0);

	xcb_change_property(info.connection, XCB_PROP_MODE_REPLACE, info.window,
		(*reply).atom, 4, 32, 1,
		&(*info.atom_wm_delete_window).atom);
	free(reply);

	xcb_map_window(info.connection, info.window);

	// Force the x/y coordinates to 100,100 results are identical in consecutive
	// runs
	const uint32_t coords[] = { 100, 100 };
	xcb_configure_window(info.connection, info.window,
		XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
	xcb_flush(info.connection);

	xcb_generic_event_t *e;
	while ((e = xcb_wait_for_event(info.connection))) {
		if ((e->response_type & ~0x80) == XCB_EXPOSE)
			break;
	}
}

void destroy_window(struct AGContext &info) {
	vkDestroySurfaceKHR(info.inst, info.surface, NULL);
	xcb_destroy_window(info.connection, info.window);
	xcb_disconnect(info.connection);
}
#endif

void init_swapchain_extension(struct AGContext &info) {
	/* DEPENDS on init_connection() and init_window() */

	VkResult U_ASSERT_ONLY res;

	// Construct the surface description:
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = info.connection;
	createInfo.hwnd = info.window;
	res = vkCreateWin32SurfaceKHR(info.inst, &createInfo,
		NULL, &info.surface);
#else  // !__ANDROID__ && !_WIN32
	VkXcbSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.connection = info.connection;
	createInfo.window = info.window;
	res = vkCreateXcbSurfaceKHR(info.inst, &createInfo,
		NULL, &info.surface);
#endif // __ANDROID__  && _WIN32
	assert(res == VK_SUCCESS);

	// Iterate over each queue to learn whether it supports presenting:
	VkBool32 *pSupportsPresent =
		(VkBool32 *)malloc(info.queue_family_count * sizeof(VkBool32));
	for (uint32_t i = 0; i < info.queue_family_count; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(info.gpus[0], i, info.surface,
			&pSupportsPresent[i]);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	info.graphics_queue_family_index = UINT32_MAX;
	info.present_queue_family_index = UINT32_MAX;
	for (uint32_t i = 0; i < info.queue_family_count; ++i) {
		if ((info.queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (info.graphics_queue_family_index == UINT32_MAX)
				info.graphics_queue_family_index = i;

			if (pSupportsPresent[i] == VK_TRUE) {
				info.graphics_queue_family_index = i;
				info.present_queue_family_index = i;
				break;
			}
		}
	}

	if (info.present_queue_family_index == UINT32_MAX) {
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (size_t i = 0; i < info.queue_family_count; ++i)
			if (pSupportsPresent[i] == VK_TRUE) {
				info.present_queue_family_index = i;
				break;
			}
	}
	free(pSupportsPresent);

	// Generate error if could not find queues that support graphics
	// and present
	if (info.graphics_queue_family_index == UINT32_MAX ||
		info.present_queue_family_index == UINT32_MAX) {
		std::cout << "Could not find a queues for both graphics and present";
		exit(-1);
	}

	// Get the list of VkFormats that are supported:
	uint32_t formatCount;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface,
		&formatCount, NULL);
	assert(res == VK_SUCCESS);
	VkSurfaceFormatKHR *surfFormats =
		(VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface,
		&formatCount, surfFormats);
	assert(res == VK_SUCCESS);
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		info.format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else {
		assert(formatCount >= 1);
		info.format = surfFormats[0].format;
	}
	free(surfFormats);
}

void init_window_size(struct AGContext &info, int32_t default_width,
	int32_t default_height) {

	info.width = default_width;
	info.height = default_height;

}

void init_command_pool(struct AGContext &info) {
	/* DEPENDS on init_swapchain_extension() */
	VkResult U_ASSERT_ONLY res;

	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = info.graphics_queue_family_index;
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	res =
		vkCreateCommandPool(info.device, &cmd_pool_info, NULL, &info.cmd_pool);
	assert(res == VK_SUCCESS);
}

void init_command_buffer(struct AGContext &info, VkCommandBuffer &newCmd) {
	/* DEPENDS on init_swapchain_extension() and init_command_pool() */
	VkResult U_ASSERT_ONLY res;

	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = info.cmd_pool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	res = vkAllocateCommandBuffers(info.device, &cmd, &newCmd);
	assert(res == VK_SUCCESS);
}
void execute_begin_command_buffer(VkCommandBuffer cmd) {
	/* DEPENDS on init_command_buffer() */
	VkResult U_ASSERT_ONLY res;

	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = NULL;

	res = vkBeginCommandBuffer(cmd, &cmd_buf_info);
	assert(res == VK_SUCCESS);
}

void execute_end_command_buffer(VkCommandBuffer cmd) {
	VkResult U_ASSERT_ONLY res;

	res = vkEndCommandBuffer(cmd);
	assert(res == VK_SUCCESS);
}

void init_device_queue(struct AGContext &info) {
	/* DEPENDS on init_swapchain_extension() */

	vkGetDeviceQueue(info.device, info.graphics_queue_family_index, 0,
		&info.graphics_queue);
	if (info.graphics_queue_family_index == info.present_queue_family_index) {
		info.present_queue = info.graphics_queue;
	}
	else {
		vkGetDeviceQueue(info.device, info.present_queue_family_index, 0,
			&info.present_queue);
	}
}

void init_swap_chain(struct AGContext &info, VkImageUsageFlags usageFlags) {
	/* DEPENDS on info.cmd and info.queue initialized */

	VkResult U_ASSERT_ONLY res;
	VkSurfaceCapabilitiesKHR surfCapabilities;

	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(info.gpus[0], info.surface,
		&surfCapabilities);
	assert(res == VK_SUCCESS);

	uint32_t presentModeCount;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(info.gpus[0], info.surface,
		&presentModeCount, NULL);
	assert(res == VK_SUCCESS);
	VkPresentModeKHR *presentModes =
		(VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
	assert(presentModes);
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(
		info.gpus[0], info.surface, &presentModeCount, presentModes);
	assert(res == VK_SUCCESS);

	VkExtent2D swapchainExtent;
	// width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
	if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = info.width;
		swapchainExtent.height = info.height;
		if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
			swapchainExtent.width = surfCapabilities.minImageExtent.width;
		}
		else if (swapchainExtent.width >
			surfCapabilities.maxImageExtent.width) {
			swapchainExtent.width = surfCapabilities.maxImageExtent.width;
		}

		if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
			swapchainExtent.height = surfCapabilities.minImageExtent.height;
		}
		else if (swapchainExtent.height >
			surfCapabilities.maxImageExtent.height) {
			swapchainExtent.height = surfCapabilities.maxImageExtent.height;
		}
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
	}

	// If mailbox mode is available, use it, as is the lowest-latency non-
	// tearing mode.  If not, try IMMEDIATE which will usually be available,
	// and is fastest (though it tears).  If not, fall back to FIFO which is
	// always available.
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < presentModeCount; i++) {
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
			(presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// Determine the number of VkImage's to use in the swap chain.
	// We need to acquire only 1 presentable image at at time.
	// Asking for minImageCount images ensures that we can acquire
	// 1 presentable image as long as we present it before attempting
	// to acquire another.
	uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfCapabilities.supportedTransforms &
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchain_ci = {};
	swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_ci.pNext = NULL;
	swapchain_ci.surface = info.surface;
	swapchain_ci.minImageCount = desiredNumberOfSwapChainImages;
	swapchain_ci.imageFormat = info.format;
	swapchain_ci.imageExtent.width = swapchainExtent.width;
	swapchain_ci.imageExtent.height = swapchainExtent.height;
	swapchain_ci.preTransform = preTransform;
	swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_ci.imageArrayLayers = 1;
	swapchain_ci.presentMode = swapchainPresentMode;
	swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
	swapchain_ci.clipped = true;
	swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchain_ci.imageUsage = usageFlags;
	swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_ci.queueFamilyIndexCount = 0;
	swapchain_ci.pQueueFamilyIndices = NULL;
	uint32_t queueFamilyIndices[2] = {
		(uint32_t)info.graphics_queue_family_index,
		(uint32_t)info.present_queue_family_index };
	if (info.graphics_queue_family_index != info.present_queue_family_index) {
		// If the graphics and present queues are from different queue families,
		// we either have to explicitly transfer ownership of images between the
		// queues, or we have to create the swapchain with imageSharingMode
		// as VK_SHARING_MODE_CONCURRENT
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_ci.queueFamilyIndexCount = 2;
		swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
	}

	res = vkCreateSwapchainKHR(info.device, &swapchain_ci, NULL,
		&info.swap_chain);
	assert(res == VK_SUCCESS);

	res = vkGetSwapchainImagesKHR(info.device, info.swap_chain,
		&info.swapchainImageCount, NULL);
	assert(res == VK_SUCCESS);

	VkImage *swapchainImages =
		(VkImage *)malloc(info.swapchainImageCount * sizeof(VkImage));
	assert(swapchainImages);
	res = vkGetSwapchainImagesKHR(info.device, info.swap_chain,
		&info.swapchainImageCount, swapchainImages);
	assert(res == VK_SUCCESS);

	for (uint32_t i = 0; i < info.swapchainImageCount; i++) {
		swap_chain_buffer sc_buffer;

		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.format = info.format;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.flags = 0;

		sc_buffer.image = swapchainImages[i];

		color_image_view.image = sc_buffer.image;

		res = vkCreateImageView(info.device, &color_image_view, NULL,
			&sc_buffer.view);
		info.buffers.push_back(sc_buffer);
		assert(res == VK_SUCCESS);
	}
	free(swapchainImages);
	info.current_buffer = 0;

	if (NULL != presentModes) {
		free(presentModes);
	}
}

void init_depth_buffer(struct AGContext &info, VkCommandBuffer cmd) {
	VkResult U_ASSERT_ONLY res;
	bool U_ASSERT_ONLY pass;
	VkImageCreateInfo image_info = {};

	/* allow custom depth formats */
	if (info.depth.format == VK_FORMAT_UNDEFINED)
		info.depth.format = VK_FORMAT_D16_UNORM;

	const VkFormat depth_format = info.depth.format;

	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(info.gpus[0], depth_format, &props);
	if (props.linearTilingFeatures &
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else if (props.optimalTilingFeatures &
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else {
		/* Try other depth formats? */
		std::cout << "depth_format " << depth_format << " Unsupported.\n";
		exit(-1);
	}

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = depth_format;
	image_info.extent.width = info.width;
	image_info.extent.height = info.height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = NUM_SAMPLES;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.image = VK_NULL_HANDLE;
	view_info.format = depth_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;

	if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT ||
		depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
		depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
		view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	VkMemoryRequirements mem_reqs;

	/* Create image */
	res = vkCreateImage(info.device, &image_info, NULL, &info.depth.image);
	assert(res == VK_SUCCESS);

	vkGetImageMemoryRequirements(info.device, info.depth.image, &mem_reqs);

	mem_alloc.allocationSize = mem_reqs.size;
	/* Use the memory properties to determine the type of memory required */
	pass = memory_type_from_properties(info, mem_reqs.memoryTypeBits,
		0, /* No requirements */
		&mem_alloc.memoryTypeIndex);
	assert(pass);

	/* Allocate memory */
	res = vkAllocateMemory(info.device, &mem_alloc, NULL, &info.depth.mem);
	assert(res == VK_SUCCESS);

	/* Bind memory */
	res = vkBindImageMemory(info.device, info.depth.image, info.depth.mem, 0);
	assert(res == VK_SUCCESS);

	/* Set the image layout to depth stencil optimal */
	set_image_layout(info, info.depth.image,
		view_info.subresourceRange.aspectMask,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		cmd);

	/* Create image view */
	view_info.image = info.depth.image;
	res = vkCreateImageView(info.device, &view_info, NULL, &info.depth.view);
	assert(res == VK_SUCCESS);
}

void init_renderpass(struct AGContext &info, bool include_depth, bool clear,
	VkImageLayout finalLayout) {
	/* DEPENDS on init_swap_chain() and init_depth_buffer() */

	VkResult U_ASSERT_ONLY res;
	/* Need attachments for render target and depth buffer */
	VkAttachmentDescription attachments[2];
	attachments[0].format = info.format;
	attachments[0].samples = NUM_SAMPLES;
	attachments[0].loadOp =
		clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = finalLayout;
	attachments[0].flags = 0;

	if (include_depth) {
		attachments[1].format = info.depth.format;
		attachments[1].samples = NUM_SAMPLES;
		attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR
			: VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].initialLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;
	}

	VkAttachmentReference color_reference = {};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = include_depth ? &depth_reference : NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = NULL;
	rp_info.attachmentCount = include_depth ? 2 : 1;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = NULL;

	res = vkCreateRenderPass(info.device, &rp_info, NULL, &info.render_pass);
	assert(res == VK_SUCCESS);
}

void init_framebuffers(struct AGContext &info, bool include_depth) {
	/* DEPENDS on init_depth_buffer(), init_renderpass() and
	* init_swapchain_extension() */

	VkResult U_ASSERT_ONLY res;
	VkImageView attachments[2];
	attachments[1] = info.depth.view;

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = NULL;
	fb_info.renderPass = info.render_pass;
	fb_info.attachmentCount = include_depth ? 2 : 1;
	fb_info.pAttachments = attachments;
	fb_info.width = info.width;
	fb_info.height = info.height;
	fb_info.layers = 1;

	uint32_t i;

	info.framebuffers = (VkFramebuffer *)malloc(info.swapchainImageCount *
		sizeof(VkFramebuffer));

	for (i = 0; i < info.swapchainImageCount; i++) {
		attachments[0] = info.buffers[i].view;
		res = vkCreateFramebuffer(info.device, &fb_info, NULL,
			&info.framebuffers[i]);
		assert(res == VK_SUCCESS);
	}
}

void init_pipeline_cache(struct AGContext &info) {
	VkResult U_ASSERT_ONLY res;

	VkPipelineCacheCreateInfo pipelineCache;
	pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCache.pNext = NULL;
	pipelineCache.initialDataSize = 0;
	pipelineCache.pInitialData = NULL;
	pipelineCache.flags = 0;
	res = vkCreatePipelineCache(info.device, &pipelineCache, NULL,
		&info.pipelineCache);
	assert(res == VK_SUCCESS);
}

void init_viewports(struct AGContext &info, VkCommandBuffer cmd) {
	info.viewport.height = (float)info.height;
	info.viewport.width = (float)info.width;
	info.viewport.minDepth = (float)0.0f;
	info.viewport.maxDepth = (float)1.0f;
	info.viewport.x = 0;
	info.viewport.y = 0;
	vkCmdSetViewport(cmd, 0, NUM_VIEWPORTS, &info.viewport);
}

void init_scissors(struct AGContext &info, VkCommandBuffer cmd) {
	info.scissor.extent.width = info.width;
	info.scissor.extent.height = info.height;
	info.scissor.offset.x = 0;
	info.scissor.offset.y = 0;
	vkCmdSetScissor(cmd, 0, NUM_SCISSORS, &info.scissor);
}


void destroy_instance(struct AGContext &info) {
	vkDestroyInstance(info.inst, NULL);
}