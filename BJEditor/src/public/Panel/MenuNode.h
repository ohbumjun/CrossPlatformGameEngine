#pragma once

// Editor 에 있는 코드이지만, Scene 에 접근할 수 있게 하기 위해
// 예외적으로 하나만 HazelEditor 가 아니라
// Hazel Editor Namespace 로 설정한다.
namespace HazelEditor
{
	class MenuNode
	{
    public:
        static MenuNode *Create(const char *rootName = nullptr);
        static void Destroy(MenuNode *);
        std::vector<MenuNode *> children;

        std::string name;
        void AddMenuItem(const char *name);
        void Draw(bool onlyDrawChildren = false);
        bool Draw(const char **name, bool onlyDrawChildren = false);
	
    
private:
        MenuNode() = default;
        MenuNode(const char *name)
            : name(name)
        {
        }

        MenuNode(std::string &&name)
            : name(std::move(name))
        {
        }
        ~MenuNode();

        bool draw(bool onlyDrawChildren);
    };
};
