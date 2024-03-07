#pragma once

#include "hzpch.h"
#include "VariableId.h"
#include "FieldInfo.h"
#include "TypeUtils.h"

// ex) 
// Transform Component 의 size 를 정상적으로 계산하기 
// 위해서 glm type 관련 header 파일 include
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


#define HAZEL_REFLECTION_FRIEND friend void ::__hazelrf_register_function(); friend struct Lv::LvMethodInfo::Result;

// 위의 메크로들은 struct 를 너무 많이 생성하기 때문에 모든 정보를 아래와 같이 그냥 담는게 나을 것 같다.

#define HAZEL_REFLECTION_REGIST									\
static void __hazelrf_register_function();							\
namespace														\
{																\
	struct __$lvrf_register__									\
	{															\
		__$hazelrf_register__()									\
		{														\
			__hazelrf_register_function();							\
		}														\
	};															\
}																\
static const __$hazelrf_register__ HAZEL_MAKE_UNIQUE_CLASS_NAME();	\
static void __lvrf_register_function()

#if defined(__HAZEL_GEN_REFLECTION__)
#define HAZEL_REFLECT __attribute__((annotate("reflect")))
#define HAZEL_ATTRIBUTE(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define HAZEL_REFLECT
#define HAZEL_ATTRIBUTE(...)
#endif


#pragma region >> example

#if 0
struct Vector3
{
	float X{}, Y{}, Z{};
};

struct Matrix3
{
	Vector3 Row0{}, Row1{}, Row2{};
};

class TranformComponent final : public GameComponent
{
private:
	Matrix3 Transform{};

	virtual void Update(float)
	{
		std::cout << "Updated\n";
	}
};

class RenderComponent final : public GameComponent
{
private:
	class Texture* Texture{};

	virtual void Render() const override
	{
		std::cout << "Rendered\n";
	}
};
int main()
{
	/*
	런타임 이전에 컴파일 하면서 각 타입에 대해서 RegisterType<TYPE> 실행
	내부적으로 static 변수 RegisterTypeOnce 를 생성.
	즉, RegisterTypeOnce()
	{
		TypeId::RegisterTypeId<T>();
	}
	이 함수는, 런타임 이전에 실행되어서, 각 타입이 등록되는 것이다.
	*/
	REGISTER_TYPE(int)
	REGISTER_TYPE(double)
	REGISTER_TYPE(Vector3)
	REGISTER_TYPE(Matrix3)
	REGISTER_TYPE(TranformComponent)
	REGISTER_TYPE(RenderComponent)

		auto& typeInfo = TypeId::GetAllTypeInformation();
	bool h = true;
}
#endif

#pragma endregion

// It's derived from MessagePack.
enum class DataType : unsigned short // uint16
{
	NIL,
	CHAR,
	UCHAR,
	BOOL,
	INT8,
	INT16,
	INT32,
	INT64,
	UINT8,
	UINT16,
	UINT32,
	UINT64,
	FLOAT,
	DOUBLE,
	STRING, 
	ARRAY,
	MAP,
	OBJECT, // OBJ_PTR
	// OBJ_STRUCT,
	POD, // ?
	BUFFER, // ?
	REFERENCE,  // -> OBJ_PTR 과 비슷..
	RAWDATA
};

enum TypeFlags
{
	TypeFlags_IsFundamental,
	TypeFlags_IsIntegral,
	TypeFlags_IsFloatingPoint,
	TypeFlags_IsTriviallyCopyable,
	TypeFlags_SIZE // MAX
};

/**
* @brief EnumField 정보 객체
*/
struct LV_API LvEnumFieldInfo
{
	/**
	* @brief 필드가 정의된 이름
	*/
	LvString name;

	/**
	* @brief 값
	*/
	uint64 value = 0;

	/**
	* @brief 필드에 정의된 Attribute 정보
	*/
	LvList<LvAttributeInfo> attributes;

	LvEnumFieldInfo() = default;

	LvEnumFieldInfo(const LvEnumFieldInfo& o) = delete;

	LvEnumFieldInfo(LvEnumFieldInfo&& o);

	~LvEnumFieldInfo() = default;
};

/**
* @brief Type정보에 부가적으로 바인딩된 속성정보
* @see https://docs.microsoft.com/ko-kr/dotnet/csharp/programming-guide/concepts/attributes/accessing-attributes-by-using-reflection
*/
struct LV_API LvAttributeInfo
{
	/**
	* @brief Attribute Type
	*/
	LvTypeId type = 0;

	/**
	* @brief Attribute 값이 선언된 대상
	*/
	LvAttributeTarget target = LvAttributeTarget::NONE;

	/**
	* @brief Attribute 객체
	*/
	LvAttribute* attribute = nullptr;

	LvAttributeInfo();

	LvAttributeInfo(const LvAttributeInfo& o);

	LvAttributeInfo(LvAttributeInfo&& o) noexcept;

	~LvAttributeInfo();

private:

	bool _hasSharedAttribute;

};

/**
* @brief 생성자 정보 객체, 일반적인 생성, 복사, 이동만 가지고 있다.
*/
struct HAZEL_API LvConstructorInfo
{
	struct LV_API UserDefine
	{
		LvList<LvParameterInfo> parameters;

		LvMethodInfo::Invoker* invoker = nullptr;
	};

	// https://en.cppreference.com/w/cpp/language/default_constructor refer eligible 
	std::function<void* (void*)> eligible;

	std::function<void* (void*, const void*)> copy;

	std::function<void* (void*, void*)> move;

	LvList<UserDefine> defines;

	LvConstructorInfo() = default;

	LvConstructorInfo(const LvConstructorInfo& o);

	LvConstructorInfo(LvConstructorInfo&& o) noexcept;

	~LvConstructorInfo() = default;
};


class GameComponent
{
public:
	GameComponent() = default;
	virtual ~GameComponent() = default;

public:
	virtual void Initialize() {};
	virtual void Update() {};
	virtual void Render() const {};
	virtual void Copy() {};
	virtual void Serialize() {};
};

class HAZEL_API Reflection
{
public :

	template<typename T>
	struct template_type_trait : std::false_type
	{
		static LvList<LvTypeId> get_template_arguments() { return {}; }
	};

	template<template <typename... > class T, typename... Args>
	struct template_type_trait<T<Args...>> : std::true_type
	{
		static LvList<LvTypeId> get_template_arguments() { return { LvReflection::GetTypeId<Args>()..., }; }
	};

	template<typename C, typename = void>
	struct has_enumerator : std::false_type {};

	template<typename C>
	struct has_enumerator<C, typename std::enable_if<
		!std::is_same<decltype(std::declval<C>().CreateEnumerator()), void>::value
	>::type> : std::true_type {};

	template<typename T>
	using is_template_instance = template_type_trait<T>;

	template<typename C, typename = void>
	struct has_type_string :std::false_type {};

	template<typename C>
	struct has_type_string<C, typename std::enable_if<
		!std::is_same<decltype(C::TypeString()), void>::value
	>::type> : std::true_type {};

	template <typename T>
	struct is_variant_primitive : std::integral_constant<bool,
		std::is_same<T, size_t>::value ||
		std::is_same<T, uint64>::value> {};

	// final : 더이상 상속하지 않는 마지막 class
	struct TypeInfo final
	{
		std::string m_Name{};
		uint32_t m_Size{};

		// alignof(T) : 해당 type 의 address 가 어떤 숫자의 배수가 되어야 하는지
		// ex) 64 bit에서는 "double" 의 aligntment 는 8이다., "int" 는 4
		// 즉, double 변수의 주소는 8의 배수여야 한다는 것이다.
		/*
			struct MyStruct {
				int x;
				double y;
			};

			alignof(MyStruct) 는 8이 된다.
			뭔가, 바이트패딩과 같이 가장 큰 자료형을 기준으로 세팅하는 것 같다.
			double 을 포함하므로 8이 된다.
			물론 이러한 alignof 값도 플랫폼, 컴파일러 등에 따라 달라진다.
		*/
		uint32_t m_Align{};

		std::bitset<TypeFlags_SIZE> m_Flags{};

		// 생성자 등록하는 방법 따로 참고하기 
		// - GameComponent 를 상속받는 Class 들은 생성자를 따로 만들어줄 수 있다.
		std::function<GameComponent* ()> m_GCConstructor{};

		std::set<FieldInfo> m_fieldInfos;

		uint32 m_PointerCount;

		/**
		* @brief 포인터 typeId
		*/
		TypeId m_PointerID = 0;

		std::vector<TypeId> m_SubTypes;

		TypeId m_Type;

		// Pointer 제외한, 원본 Type
		TypeId m_OriginalType;
		
		bool m_IsArray = false;

		bool m_IsIterable = false;

		bool m_IsPrimitive = false;

		bool m_IsPod = false;

		bool m_IsEnum = false;
	};

	template <typename T>
	static std::string GetTypeName();

	template <>
	static std::string GetTypeName<void>(){return "void";}

	static std::string GetTypeName(const TypeId& id);

	static uint64_t Hash(std::string_view str);
	
	template <typename T, typename std::enable_if<std::is_array<T>::value, T>::type* = nullptr>
	static TypeId GetPointerTypeId()
	{
		return GetTypeID<typename remove_all<T>::type*>();
	}

	template <typename T, typename std::enable_if<!std::is_array<T>::value, T>::type* = nullptr>
	static TypeId GetPointerTypeId()
	{
		return GetTypeID<T*>();
	}

	static TypeId GetPointerTypeId(TypeId typeId)
	{
		return GetTypeInfo(typeId)->m_PointerID;
	}

	/**
	* const T 를 반환
	*/
	template <typename T, typename std::enable_if<!std::is_void<T>::value, T>::type* = nullptr>
	static TypeId GetConstTypeId()
	{
		static_assert(!std::is_const<T>::value, "T is already const");
		return GetTypeID<const T>();
	}

	/**
	* void 타입일 경우 invalid type 을 반환
	*/
	template <typename T, typename std::enable_if<std::is_void<T>::value, T>::type* = nullptr>
	static TypeId GetConstTypeId()
	{
		return 0;
	}

	/*
	template <typename T, typename std::enable_if<!std::is_void<T>::value, T>::type* = nullptr>
	static TypeId GetTypeID() noexcept
	{
		//static char const type_id = '\0';
		//return &type_id;
		static uint32 type_id = 0;

		if (type_id == 0)
		{
			LvString name = MakeTypeString<T>();
			type_id = MakeTypeId(name.c_str());
		}

		return type_id;
	}

	template <typename T, typename std::enable_if<std::is_void<T>::value, T>::type* = nullptr>
	static TypeId GetTypeID() noexcept
	{
		return 0;
	}

	template <typename T>
	static TypeId GetTypeID(const T& t)
	{
		return GetTypeID<typename remove_all<T>::type>();
	}

	*/

	/**
	* 탬플릿 가변 인자 ts 의 Type 배열을 반환
	*/
	template <typename... Ts>
	static std::vector<TypeId> GetTypeIDs()
	{
		std::vector<TypeId> r;
		r.push_back(GetTypeID<Ts>()...);
		return r;
	}

	template<typename T>
	static TypeId GetTypeID();

	template<typename T>
	static TypeId RegistType();

	template<typename T>
	static bool IsRegistered();

	static bool IsRegistered(const TypeId& id);

	template<typename Base, typename Sub>
	static void RegisterBase();

	static void RegisterBase(TypeId base, TypeId subType);

	// static void RegistTypeCode(const LvTypeId type, const TypeCode code);
	static void RegistDataType(const LvTypeId type, const DataType code);

	static uint32 GetTypeSize(const TypeId& id);

	static uint32 GetTypeAlignment(TypeId id);

	template<typename T>
	static TypeInfo* GetTypeInfo();

	static TypeInfo* GetTypeInfo(const TypeId& id);

	static const auto& GetAllTypeInformation();

	static DataType GetDataType(TypeId id);

	static bool IsFundamental(TypeId id)	;
	static bool IsIntegral(TypeId id)	;
	static bool IsFloatingPoint(TypeId id) ;
	static bool IsTriviallyCopyable(TypeId id) ;

	static void Regist(const TypeId type, TypeInfo&& info);

	template <typename Class, typename Field>
	static FieldInfo RegisterField(const std::string& fieldName, uint32_t Offset);

	template<typename Field>
	static FieldInfo RegisterField(TypeId classId, const std::string& fieldName, uint32_t Offset);

	static FieldInfo RegisterField(TypeId classId, VariableId FieldId, const std::string& fieldName, uint32_t Offset, uint32_t Size, uint32_t Align, bool isIterable, bool isArray);
	
	template<typename T, typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr>
	static void RegistEnum(TypeInfo& info)
	{
		// RegistTypeCode(info.id, Reflection::GetTypeCode(LV_TYPEOF(typename std::underlying_type<T>::type)));
		RegistTypeCode(info.id, Reflection::GetDataType(LV_TYPEOF(typename std::underlying_type<T>::type)));
	}

	template<typename T, typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
	static void RegistEnum(TypeInfo& info)
	{

	}

	/**
	* @brief 탬플릿 인자 TEnum 의 Type 정보에 Enum 필드를 등록
	*/
	template<typename TEnum>
	static void RegistEnumField(const char* fieldName, size_t value = 0)
	{
		Regist<TEnum>();

		LvEnumFieldInfo info;
		info.name = fieldName;
		info.value = value;

		LvTypeId type = GetTypeId<TEnum>();

		RegistEnumField(type, std::move(info));
	}
	
	/**
	* @brief type에 EnumField 정보를 등록
	*/
	static void RegistEnumField(const LvTypeId type, LvEnumFieldInfo&& info);

	static bool CanRegist(const TypeId& type)
	{
		// const TypeInfoContainer& container = GetContainers();
		// return !container.Contains(type);	
		return false;
	}

	/**
	* 등록된 Type을 Delete
	*/
	static void Unregist(const TypeId type)
	{
		// TypeInfoContainer& container = GetContainers();
		// LvHashtable<LvString, LvTypeId>& nameMap = GetNameMaps();
		// if (container.Contains(type))
		// {
		// 	const LvTypeInfo& info = container[type];
		// 	if (nameMap.ContainsKey(info.name))
		// 	{
		// 		nameMap.Remove(info.name);
		// 	}
		// 	container.Remove(type);
		// }
	}

	/**
	* @brief 탬플릿 인자 T를 name 인자로 Type 정보를 등록합니다.
	* @param assemblyName dll 이름
	* @param refresh 재등록 여부
	*/
	template <typename T>
	static TypeId RegistByName(const char* name, const char* assemblyName = "", bool refresh = false)
	{
		TypeId id = GetTypeID<T>();

		if (!CanRegist(id))
		{
			if (refresh)
				Unregist(id);
			else
				return id;
		}

		TypeInfo info;
		// info.size = sizeof(T);
		// info.align = LV_ALIGNOF(T);
		// info.id = id;
		// info.assemblyName = assemblyName;
		// info.isArray = std::is_array<T>::value;
		// info.isIterable = is_iterable<T>::value;
		// info.isPrimitive = std::is_fundamental<T>::value;
		// info.isPod = std::is_pod<T>::value;
		// info.isEnum = std::is_enum<T>::value;
		// info.rawId = GetTypeID<typename remove_all<T>::type>();
		// info.pointerCount = pointer_count<T>::value;
		// info.name = name;
		// info.templateArguments = template_type_trait<T>::get_template_arguments();
		// info.pointerId = GetPointerTypeId<T>();

		if (!info.isArray)
		{
			RegistPointer<T*>(assemblyName, refresh);
		}
		
		RegistEnum<T>(info);
		RegistConstructors<T>(info);
		Regist(id, std::move(info));
		RegistEnumerator<T>(info);

		return id;
	}

	/**
	* @brief 탬플릿 인자 T 값을 등록합니다.
	* @param assemblyName dll 이름
	* @param refresh 재등록 여부
	*/
	template <typename T>
	static TypeId Regist(const char* assemblyName = "", bool refresh = false)
	{
		return RegistByName<T>(TypeRawName<T>().c_str(), assemblyName, refresh);
	}

	/**
	* @brief 기본/소멸자들을 등록합니다.
	*/
	template <typename T, 
		typename std::enable_if<
			std::is_class<T>::value && !std::is_abstract<T>::value, T
	>::type* = nullptr>
	static void RegistConstructors(TypeInfo& info)
	{
		RegistDefaultConstructor<T>(info);

		RegistMoveConstructor<T>(info);

		RegistCopyConstructor<T>(info);

		RegistDeconstructor<T>(info);
	}

	/**
	* @brief 기본/소멸자들이 없을 경우.
	*/
	template <typename T,
		typename std::enable_if<
		!std::is_class<T>::value || std::is_abstract<T>::value, T
	>::type* = nullptr>
	static void RegistConstructors(TypeInfo& info) { }

	/**
	* @brief 기본 생성자가 있을 경우.
	*/
	template<typename T, 
		typename std::enable_if<
		std::is_default_constructible<T>::value || 
		std::is_trivially_default_constructible<T>::value, T
	>::type* = nullptr>
	static void RegistDefaultConstructor(TypeInfo& info)
	{
		info.constructors.eligible = [](void* ptr)
		{
			return new (ptr) T();
		};
	}
	/**
	* @brief 기본 생성자가 없을 경우.
	*/
	template<typename T,
		typename std::enable_if<
		!(std::is_default_constructible<T>::value || 
		  std::is_trivially_default_constructible<T>::value), T
	>::type* = nullptr>
	static void RegistDefaultConstructor(TypeInfo& info) { }

	/**
	* @brief 이동 생성자가 있을 경우.
	*/
	template<typename T,
		typename std::enable_if<
		(std::is_move_constructible<T>::value ||
		std::is_trivially_move_constructible<T>::value) && !std::is_pointer<T>::value, T
	>::type* = nullptr>
	static void RegistMoveConstructor(TypeInfo& info)
	{
		info.constructors.move = [](void* ptr, void* other)
		{
			T* t = static_cast<T*>(other);
			T* res = new (ptr) T(std::move(*t));
			t->~T();
			return res;
			//return new (ptr) T(std::move(*t));
		};
	}

	/**
	* @brief 이동 생성자가 없을 경우.
	*/
	template<typename T,
		typename std::enable_if<
		!(std::is_move_constructible<T>::value ||
		std::is_trivially_move_constructible<T>::value || std::is_pointer<T>::value), T
	>::type* = nullptr>
	static void RegistMoveConstructor(TypeInfo& info) { }

	/**
	* @brief 복사 생성자가 있을 경우.
	*/
	template<typename T,
		typename std::enable_if<
		std::is_copy_constructible<T>::value && !std::is_pointer<T>::value, T
	>::type* = nullptr>
	static void RegistCopyConstructor(TypeInfo& info)
	{
		info.constructors.copy = [](void* dst, const void* src)
		{
			const T* t = static_cast<const T*>(src);
			return new (dst) T(*t);
		};
	}

	/**
	* @brief 복사 생성자가 없을 경우.
	*/
	template<typename T,
		typename std::enable_if<
		!std::is_copy_constructible<T>::value || std::is_pointer<T>::value, T
	>::type* = nullptr>
	static void RegistCopyConstructor(TypeInfo& info) { }

	/**
	* @brief 소멸자가 있을 경우.
	*/
	template<typename T,
		typename std::enable_if<
		std::is_destructible<T>::value, T
	>::type* = nullptr>
	static void RegistDeconstructor(TypeInfo& info)
	{
		info.destructor = [](void* ptr)
		{
			T* t = static_cast<T*>(ptr);
			t->~T();
		};
	}

	static void RegistConstructor(const TypeId& type, LvConstructorInfo::UserDefine&& info);

	/**
	* @brief 소멸자가 없을 경우.
	*/
	template<typename T,
		typename std::enable_if<
		!std::is_destructible<T>::value, T
	>::type* = nullptr>
	static void RegistDeconstructor(TypeInfo& info) { }

	static size_t GetFieldCount(TypeId classId);

	static const std::set<FieldInfo>* GetFieldInfos(TypeId classId);

	static const FieldInfo* GetFieldInfo(TypeId id, const std::string& FieldName);

	static bool IsDerivedClass(TypeId baseType, TypeId childType);
private :

	class StaticContainerData
	{
		friend class Reflection;

	private :
		StaticContainerData();

		template<typename T>
		void registDataType(DataType code);

		// TypeId 의 Id ~ TypeInfo
		std::unordered_map<TypeId, TypeInfo>	typeInfos{};
		std::unordered_map<TypeId, DataType>	typeDataCodeMap{};

		// // Class Type -> (member field 이름, member offset)
		// std::unordered_map<TypeId, std::unordered_map<std::string, uint32_t>> FieldInfoNameMap{};

		/*
			template<typename T, typename std::enable_if<!std::is_const<T>::value, T>::type* = nullptr>
	void Regist(LvReflection::TypeCode code, bool isPrimitive)
	{
		const LvTypeId id = LvReflection::GetTypeId<T>();
		
		LvTypeInfo info;
		info.size = sizeof(T);
		info.align = LV_ALIGNOF(T);
		info.id = id;
		info.assemblyName = __ASSEMBLY__NAME__;
		info.isArray = std::is_array<T>::value;
		info.isIterable = is_iterable<T>::value;
		info.isPrimitive = isPrimitive;
		info.isPod = std::is_pod<T>::value;
		info.isEnum = std::is_enum<T>::value;
		info.rawId = LvReflection::GetTypeId<typename remove_all<T>::type>();
		info.pointerCount = pointer_count<T>::value;
		info.name = LvReflection::MakeTypeString<T>();
		info.templateArguments = LvReflection::template_type_trait<T>::get_template_arguments();
		info.pointerId = LvReflection::GetPointerTypeId<T>();

		LvReflection::RegistConstructors<T>(info);
		Regist(id, std::move(info));
		
		_typeCodeMap.Add(id, code);

		Regist<const T>(code, isPrimitive);
	}

	template<typename T, typename std::enable_if<std::is_const<T>::value, T>::type* = nullptr>
	void Regist(LvReflection::TypeCode code, bool isPrimitive)
	{
		const LvTypeId id = LvReflection::GetTypeId<T>();

		LvTypeInfo info;
		info.size = sizeof(T);
		info.align = LV_ALIGNOF(T);
		info.id = id;
		info.assemblyName = __ASSEMBLY__NAME__;
		info.isArray = std::is_array<T>::value;
		info.isIterable = is_iterable<T>::value;
		info.isPrimitive = isPrimitive;
		info.isPod = std::is_pod<T>::value;
		info.isEnum = std::is_enum<T>::value;
		info.rawId = LvReflection::GetTypeId<typename remove_all<T>::type>();
		info.pointerCount = pointer_count<T>::value;
		info.name = LvReflection::MakeTypeString<T>();
		info.templateArguments = LvReflection::template_type_trait<T>::get_template_arguments();
		// @donghun 파싱 중 예외 발생으로 인해 해당 부분은 주석처리 하였으나 처리 필요.
		// info.pointerId = LvReflection::GetPointerTypeId<T>();

		//LvReflection::RegistConstructor<T>(info);
		Regist(id, std::move(info));

		_typeCodeMap.Add(id, code);
	}

	// TODO : 중복 코드이긴 한데 방법이 딱히 없네 ;
	void Regist(const LvTypeId type, LvTypeInfo&& info) const
	{
		TypeInfoContainer& container = GetContainers();
		LvHashtable<LvString, LvTypeId>& nameMap = GetNameMaps();

		if (!nameMap.ContainsKey(info.name))
		{
			nameMap.Add(info.name, type);
		}

		if (!container.Contains(type))
		{
			container.Add(type, std::move(info));
		}
	}

	void RegistTypeCode(const LvTypeId type, const LvReflection::TypeCode typecode)
	{
		if (!_typeCodeMap.ContainsKey(type))
		{
			_typeCodeMap.Add(type, typecode);
		}
	}

	LvReflection::TypeCode GetTypeCode(LvTypeId type) const
	{
		if (false == LvReflection::HasRegist(type))
		{
			return LvReflection::TypeCode::EMPTY;
		}

		if (_typeCodeMap.ContainsKey(type))
		{
			return _typeCodeMap[type];
		}

		const LvTypeInfo* info = LvReflection::GetTypeInfo(type);

		if (info->isEnum)
		{
#if defined(UINT64)
#undef UINT64
#endif 
			return LvReflection::TypeCode::UINT64;
		}

		return LvReflection::TypeCode::OBJECT;
	}
		*/
	};

	static StaticContainerData& getStaticContainerData();

	// 아래 함수들을 멀티쓰레드 환경에서 동기화가 필요한 함수들일것 같다..
	template <typename T>
	static std::string createTypeName();

	template<typename T>
	static TypeId createTypeID();

	template<typename T>
	static TypeInfo createTypeInfo();
};

//  이 함수는 말 그대로 TypeInfo 에서 Name 을 리턴하도록 수정하고
// 해당 함수의 역할은, MakeTypeName 등 다른 함수로 대체한다.
template <typename T>
inline std::string Reflection::GetTypeName()
{
	static TypeId typeID = 0;

	if (typeID == (uint64_t)0)
	{
		typeID = GetTypeID<T>();
	}

	return GetTypeName(typeID);
}


template<typename T>
inline TypeId Reflection::RegistType()
{
	static TypeId typeId = 0;

	if (typeId == (uint64)0)
	{
		const TypeInfo& typeInfo = createTypeInfo<T>();
		typeId = typeInfo.m_Type;
	}

	return typeId;
};

template<typename T>
inline TypeId Reflection::GetTypeID()
{
	static TypeId typeID = 0;

	if (typeID == (uint64_t)0)
	{
		typeID = createTypeID<T>();
	}

	return typeID;
}

template<typename T>
inline bool Reflection::IsRegistered()
{
	static StaticContainerData& containerData = getStaticContainerData();

	static TypeId id = GetTypeID<T>();

	bool isContainId = containerData.typeInfos.find(id) != containerData.typeInfos.end();

	return isContainId;
}

template<typename Base, typename Sub>
inline void Reflection::RegisterBase()
{
	TypeId baseType = GetTypeID<Base>();
	TypeId subType	= GetTypeID<Sub>();

	TypeInfo* baseTypeInfo = GetTypeInfo(baseType);

	bool contain = std::find(baseTypeInfo->subTypes.begin(), baseTypeInfo->subTypes.end(), subType) != baseTypeInfo->subTypes.end();

	if (contain) return;

	baseTypeInfo->subTypes.push_back(subType);
}

template<typename T>
inline Reflection::TypeInfo* Reflection::GetTypeInfo()
{
	static StaticContainerData& containerData = getStaticContainerData();

	static TypeId id = GetTypeID<T>();

	bool isContainId = containerData.typeInfos.find(id) != containerData.typeInfos.end();

	if (isContainId == false)
	{
		createTypeInfo<T>();
	}

	return &containerData.typeInfos[id];
}

/*private functions*/

template <typename T>
inline std::string Reflection::createTypeName()
{
	// T 가 int 일 경우 class std::basic_string_view<char,struct std::char_traits<char> > __cdecl Reflection::detail::wrapped_type_name<int>(void)
	auto wrapped_name = TypeUtils::wrapped_type_name<T>();

	// wrapped_type_name<type_name_prober>() ?
	//  : wrapped_type_name<type_name_prober>() : class std::basic_string_view<char,struct std::char_traits<char> > __cdecl Reflection::detail::wrapped_type_name<void>(void)
	// 결과 : 112
	 auto prefix_length = TypeUtils::wrapped_type_name_prefix_length();

	// 결과 : 7
	 auto suffix_length = TypeUtils::wrapped_type_name_suffix_length();

	// wrapped_name 에서 <> 안에 있는 "int" 만큼의 길이 == 3
	auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;

	auto typeName = wrapped_name.substr(prefix_length, type_name_length);

	// <> 안에 있는 "int" 의 시"
	return typeName;
}

template<typename T>
inline TypeId Reflection::createTypeID()
{
	static uint64_t typeID = 0;

	if (typeID == (uint64)0)
	{
		uint64_t hashString = Hash(createTypeName<T>());

		typeID = hashString;
	}

	return TypeId(typeID);
}

template<typename T>
inline Reflection::TypeInfo Reflection::createTypeInfo()
{
	static StaticContainerData& containerData = getStaticContainerData();

	TypeId typeId = GetTypeID<T>();

	bool isContainId = containerData.typeInfos.find(typeId) != containerData.typeInfos.end();

	if (isContainId)
	{
		return containerData.typeInfos[typeId];
	}

	TypeInfo info{};

	info.m_Name = createTypeName<T>();
	info.m_Type = typeId;
	info.m_Align = 0;
	info.m_Size = 0;

	if constexpr (!std::is_same_v<void, T>)
	{
		info.m_Size = sizeof(T);
		info.m_Align = alignof(T);
	}

	// memcpy  와 같은 함수를 통해서 copy 가 가능하면 trivially_copyable
	/*
	false 인 경우 ?

	struct NonTriviallyCopyable {
		NonTriviallyCopyable() {}
		NonTriviallyCopyable(const NonTriviallyCopyable&) {}
		~NonTriviallyCopyable() {}
	};

	userdefined constructor, copy constructor, and destructor 가 있는 경우
	혹은 그러한 멤버를 들고 있는 경우
	*/
	info.m_Flags[TypeFlags_IsTriviallyCopyable] = std::is_trivially_copyable_v<T>;
	info.m_Flags[TypeFlags_IsIntegral] = std::is_integral_v<T>;
	info.m_Flags[TypeFlags_IsFloatingPoint] = std::is_floating_point_v<T>;
	info.m_Flags[TypeFlags_IsFundamental] = std::is_fundamental_v<T>;

	// 모든 array extent 제거 ex) int[2][3] -> int
	using Type_RemovedExtents = std::remove_all_extents_t<T>;

	// &, && 제거
	// ex) int, bool 같은 primitive type 에 대해 진행하면 아무 효과 x
	using Type_RemovedRefs = std::remove_reference_t<Type_RemovedExtents>;

	// * 제거
	using Type_RemovedPtrs_And_Refs = remove_all_pointers_t<Type_RemovedRefs>;

	// const , volatile 제거 
	// using StrippedType = std::remove_cvref_t<Type_RemovedPtrs>;
	using StrippedType = std::remove_cv_t<Type_RemovedPtrs_And_Refs>;

	// 몇개의 pointer 가 있는지 계산한다.
	info.m_PointerCount = TypeUtils::CountPointers<Type_RemovedRefs>();

	if (info.m_PointerCount > 0)
	{
		info.m_OriginalType = GetTypeID<Type_RemovedPtrs_And_Refs>();
	}
	else
	{
		info.m_OriginalType = info.m_Type;
	}

	// GameComponent 를 상속받은 대상이라면 생성자 함수를 세팅해준다. 
	if constexpr (std::is_base_of_v<GameComponent, T>)
	{
		info.m_GCConstructor = []()->GameComponent*
		{
			return new T();
		};
	}

	containerData.typeInfos.insert(std::make_pair(typeId, info));

	return info;
}


// VARNAME##TYPE 을 붙인 이름의 "변수"로 만든다.
// #define _REGISTER_TYPE_INTERNAL(TYPE,VARNAME) RegistType<TYPE> VARNAME##TYPE{};
// #define REGISTER_TYPE(TYPE) _REGISTER_TYPE_INTERNAL(TYPE, RegistType_)

// REGISTER_TYPE(int) ==
// RegisterType<int>RegisterType_int{}

// REGISTER_TYPE(int) 에 해당하는 코드를, 각 Class 별로 모두 작성해줘야 한다.
// main.cpp 실행 전에 작성해줘도 되고, 그것은 알아서
// 영상에서는 main.cpp 에 바로 적어줌
// 어차피 static 변수 형태이므로, 어디에 적어줘도 상관없다.

