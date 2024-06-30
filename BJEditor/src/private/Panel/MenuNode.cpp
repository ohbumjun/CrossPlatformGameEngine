#include "hzpch.h"
#include "Panel/MenuNode.h"
#include "imgui.h"
#include "Hazel/Core/Allocation/Allocator/FreeListAllocator.h"

namespace HazelEditor
{
    static Hazel::FreeListAllocator s_FreeListAllocator(1024, 4);
static int64 count = 0;

MenuNode *MenuNode::Create(const char *rootName)
{
    uint64 rootNameLength = nullptr != rootName ? strlen(rootName) : 0;
    void* menuNodePtr = s_FreeListAllocator.Allocate(sizeof(MenuNode));
    MenuNode *root = new (menuNodePtr) MenuNode("root");

    std::vector<std::string> itemNameBuffer;

    // 여기에 Menu 에 표시할 대상을 넣어준다.
    // ex) itemNameBuffer.push_back("File/New Scene");

    for (const std::string& itemName : itemNameBuffer)
	{
		root->AddMenuItem(itemName.c_str());
	}
    
    // LvFixedString<LV_CHAR_INIT_LENGTH> itemNameBuffer;
    // for (uint64 i = 0, max = attributes.Count(); i < max; ++i)
    // {
    //     if (nullptr != rootName)
    //     {
    //         // attributes[i]->name은 "rootName/item name"을 갖으므로 다음과 같이 "rootName/"을 제외하고 itemNameBuffer에 넣어준다.
    //         const char *attributeNameWithoutRootName =
    //             attributes[i]->name.c_str() + rootNameLength + 1;
    //         const uint32 attributeNameWithoutRootNameLen =
    //             static_cast<uint32>(strlen(attributes[i]->name.c_str())) -
    //             (static_cast<uint32>(rootNameLength) + 1);
    // 
    //         itemNameBuffer.Clear();
    //         itemNameBuffer.Append(attributeNameWithoutRootName,
    //                               0,
    //                               attributeNameWithoutRootNameLen);
    //         root->AddMenuItem(itemNameBuffer.c_str());
    //     }
    //     else
    //     {
    //         root->AddMenuItem(attributes[i]);
    //     }
    // }

    return root;
}

void MenuNode::Destroy(MenuNode *node)
{
    // LV_LOG(warning, "[Free] %u / %s", --count, node->name.c_str());
    node->~MenuNode();
    s_FreeListAllocator.Free(node);
}


void MenuNode::AddMenuItem(const char *name)
{
    HZ_ASSERT(!(nullptr == name || 0 == strcmp(name, "")), "Empty name");

    // Find Slash in the name characters
    int firstSlashIndex = -1;
    for (int i = 0; name[i] != '\0'; ++i)
    {
        if (name[i] == '/')
        {
            firstSlashIndex = i;
            break;
        }
    }

    if (firstSlashIndex == -1)
    {
        // name parameter에 slash가 없는 경우
        children.push_back(new (s_FreeListAllocator.Allocate(sizeof(MenuNode))) MenuNode(name));
    }
    else
    {
        // name parameter에 slash가 있는 경우
        MenuNode *node = nullptr;
        // int index = children.Find([name, firstSlashIndex](MenuNode *node) {
        //     return (0 == Hazel::pr_strncmp(node->name.c_str(), name, firstSlashIndex));
        // });
        std::string targetName(name);
        auto it =
            std::find_if(children.begin(),
                         children.end(),
                         [&targetName](MenuNode *node) {
                             return std::strncmp(node->name.c_str(),
                                                 targetName.c_str(),
                                                 targetName.length()) == 0;
                         });
        int index = it != children.end() ? std::distance(children.begin(), it) : -1;

        if (-1 == index)
        {
            std::string currentName;
            currentName.append(name, 0, firstSlashIndex);
            node = new (s_FreeListAllocator.Allocate(sizeof(MenuNode))) MenuNode(std::move(currentName));
            // LV_LOG(warning, "[Alloc] %u / %s", ++count, node->name.c_str());
            children.push_back(node);
        }
        else
        {
            node = children[index];
        }

        // first slash이후의 name character로 바꾼 후 AddMenuItem
        name = name + firstSlashIndex + 1;

        node->AddMenuItem(name);
    }
}

void MenuNode::Draw(bool onlyDrawChildren)
{
    if (draw(onlyDrawChildren))
    {
        // if (find->useParameterOnClick)
        // {
        //     if (nullptr != find->onClick)
        //     {
        //         find->onClick(find->name.c_str());
        //     }
        //     else
        //     {
        //         LV_LOG(warning, "Callback is nullptr (%s)", find->name.c_str());
        //     }
        // }
        // else
        // {
        //     if (nullptr != find->onClickWithoutParameter)
        //     {
        //         find->onClickWithoutParameter();
        //     }
        //     else
        //     {
        //         LV_LOG(warning, "Callback is nullptr (%s)", find->name.c_str());
        //     }
        // }
    }
}

bool MenuNode::Draw(const char **name, bool onlyDrawChildren)
{
    if (draw(onlyDrawChildren))
    {
        // if (find->useParameterOnClick)
        // {
        //     *name = find->name.c_str();
        //     if (nullptr == find->onClick)
        //     {
        //         LV_LOG(warning, "Callback is nullptr (%s)", find->name.c_str());
        //     }
        // }
        // else
        // {
        //     *name = find->name.c_str();
        //     if (nullptr == find->onClickWithoutParameter)
        //     {
        //         LV_LOG(warning, "Callback is nullptr (%s)", find->name.c_str());
        //     }
        // }
        return true;
    }
    return false;
}

MenuNode::~MenuNode()
{
    for (uint64 i = 0, max = children.size(); i < max; i++)
    {
        Destroy(children[i]);
    }
}

bool MenuNode::draw(bool onlyDrawChildren)
{
    bool result = false;
    if (children.empty())
    {
        bool enabled = true;
        // if (attribute->useParameterOnCheck)
        // {
        //     if (nullptr != attribute->onCheck)
        //     {
        //         enabled = attribute->onCheck(attribute->name.c_str());
        //     }
        // }
        // else
        // {
        //     if (nullptr != attribute->onCheckWithoutParameter)
        //     {
        //         enabled = attribute->onCheckWithoutParameter();
        //     }
        // }

        if (ImGui::MenuItem(name.c_str(), nullptr, nullptr, enabled))
        {
            result = true;
        }
    }
    else
    {
        if (onlyDrawChildren || ImGui::BeginMenu(name.c_str()))
        {
            for (uint64 i = 0, max = children.size(); i < max; i++)
            {
                const int prev = static_cast<int>(i - 1);

                // if (0 <= prev && children[i] &&
                //     nullptr != children[prev]->attribute &&
                //     nullptr != children[i]->attribute)
                // {
                //     if (10 <= children[i]->attribute->priority -
                //                   children[prev]->attribute->priority)
                //     {
                //         ImGui::Separator();
                //     }
                // }

               ImGui::Separator();

                result |= children[i]->draw(false);
            }

            if (!onlyDrawChildren)
            {
                ImGui::EndMenu();
            }
        }
    }
    return result;
}

} // namespace HazelEditor
