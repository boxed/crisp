#pragma once

struct GRuntimeClass
{
	LPCSTR m_lpszClassName;
	int m_nObjectSize;
	GRuntimeClass* m_pBaseClass;
	GRuntimeClass* m_pNextClass;       // linked list of registered classes

	bool IsDerivedFrom(const GRuntimeClass* pBaseClass) const
	{
		// simple SI case
		const GRuntimeClass* pClassThis = this;
		while(pClassThis != NULL)
		{
			if(pClassThis == pBaseClass)
				return true;
			pClassThis = pClassThis->m_pBaseClass;
		}
		return false;       // walked to the top, no match
	}
};

#define G_RUNTIME_CLASS(class_name) \
	((GRuntimeClass*)(&class_name::class##class_name))

#define G_DECLARE_DYNAMIC(class_name) \
public: \
	static const GRuntimeClass class##class_name; \
	virtual GRuntimeClass* GetRuntimeClass() const; \

#define G_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, class_init) \
	const GRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), \
		G_RUNTIME_CLASS(base_class_name), class_init }; \
	GRuntimeClass* class_name::GetRuntimeClass() const \
		{ return G_RUNTIME_CLASS(class_name); } \

#define G_IMPLEMENT_DYNAMIC(class_name, base_class_name) \
	G_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, NULL)
