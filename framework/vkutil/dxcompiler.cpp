#include "framework.h"

#include "functions.h"
#include "shaderutil.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)

struct IUnknown;

#include <windows.h>
#include <combaseapi.h>
#include <dxc/dxcapi.h>

namespace vkutil
{

    static const wchar_t* g_dxcProfile[]
    {
        L"vs_6_0", L"ps_6_0"
    };

    static const wchar_t* g_dxcEntryPoint[]
    {
        L"VSMain", L"FSMain"
    };

    class DXCCompiler : public IHLSLCompiler, private Functions
    {

    public:

        virtual bool Initialize(VkDevice device, VkAllocationCallbacks* pVkAlloc) override
        {
            HMODULE dll = LoadLibrary("dxcompiler.dll");
            if (dll == nullptr) 
                VKUTIL_CHECK_RETURN(VK_ERROR_UNKNOWN, false);            
            DxcCreateInstanceProc DxcCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(GetProcAddress(dll, "DxcCreateInstance"));
            if (!DxcCreateInstance)
                VKUTIL_CHECK_RETURN(VK_ERROR_UNKNOWN, false);
            if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_pDxcUtils))))
                VKUTIL_CHECK_RETURN(VK_ERROR_UNKNOWN, false);
            if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_pDxcCompiler))))
                VKUTIL_CHECK_RETURN(VK_ERROR_UNKNOWN, false);
            if (FAILED(m_pDxcUtils->CreateDefaultIncludeHandler(&m_pDxcIncludeHandler)))
                VKUTIL_CHECK_RETURN(VK_ERROR_UNKNOWN, false);
            m_pVkAlloc = pVkAlloc;
            m_vkDevice = device;
            return true;
        }

        virtual bool Compile(const HLSLCode& code, VkShaderModule* pShaderModule) override
        {
            const wchar_t* options[]
            {
                L"-T", g_dxcProfile[code.stage],
                L"-spirv", L"-fspv-target-env=vulkan1.1",
                L"-E", g_dxcEntryPoint[code.stage],
                L"-fvk-invert-y"
            };            

            IDxcBlobEncoding* pSrcBlob = nullptr;
            if (FAILED(m_pDxcUtils->CreateBlobFromPinned(code.pCode, code.codeSize, CP_UTF8, &pSrcBlob)))
                VKUTIL_CHECK_RETURN(VK_ERROR_UNKNOWN, false);
            DxcBuffer source
            { 
                pSrcBlob->GetBufferPointer(), 
                pSrcBlob->GetBufferSize(), 
                CP_UTF8 
            };

            bool retVal = false;
            IDxcResult* pCompileResult = nullptr;
            uint32_t numOpts = _countof(options) - ((code.stage != VERTEX_SHADER) ? 1 : 0);
            HRESULT result = m_pDxcCompiler->Compile(&source, options, numOpts, m_pDxcIncludeHandler, IID_PPV_ARGS(&pCompileResult));
            if (SUCCEEDED(result))
            {
                IDxcBlob* pByteCode = nullptr;
                result = pCompileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pByteCode), nullptr);
                if (SUCCEEDED(result) && pByteCode && pByteCode->GetBufferSize())
                {
                    VkShaderModuleCreateInfo info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
                    info.codeSize = pByteCode->GetBufferSize() & UINT32_MAX;
                    info.pCode = reinterpret_cast<uint32_t*>(pByteCode->GetBufferPointer());
                    retVal = (vkCreateShaderModule(m_vkDevice, &info, m_pVkAlloc, pShaderModule) == VK_SUCCESS);
                    pByteCode->Release();
                    retVal = true;
                }
            }
            
            if (!retVal)
            {
                IDxcBlobUtf8* pErrors = nullptr;
                result = pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
                if (SUCCEEDED(result) && pErrors && pErrors->GetStringLength())
                    OutputDebugString(pErrors->GetStringPointer());
                if (pErrors)
                    pErrors->Release();
            }

            if (pCompileResult)
                pCompileResult->Release();
            pSrcBlob->Release();
            return retVal;
        }

        virtual void Finalize() override
        {
            m_pDxcIncludeHandler->Release();
            m_pDxcCompiler->Release();
            m_pDxcUtils->Release();
        }

    private:

        VkDevice m_vkDevice = VK_NULL_HANDLE;
        VkAllocationCallbacks* m_pVkAlloc = nullptr;

        IDxcIncludeHandler* m_pDxcIncludeHandler = nullptr;
        IDxcCompiler3* m_pDxcCompiler = nullptr;
        IDxcUtils* m_pDxcUtils = nullptr;

    };

    IHLSLCompiler* IHLSLCompiler::Get()
    {
        static DXCCompiler compiler;
        return &compiler;
    }

}

#endif
