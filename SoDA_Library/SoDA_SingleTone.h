#ifndef _SODA_SINGLETONE_H_
#define _SODA_SINGLETONE_H_

//#include <iostream>

using namespace std;

namespace SoDA {
	class Singletone {
	private :
		static Singletone* pInstance;
		Singletone() {}
		~Singletone() {}

	public :
		static Singletone* getInstance()
		{
			if (NULL == pInstance) {
				pInstance = new Singletone();
			}

			return pInstance;
		}

		static void freeInstance()
		{
			if (NULL != pInstance) {
				delete pInstance;
				pInstance = NULL;
			}
		}
	};	// class Singletone

	Singletone* Singletone::pInstance = NULL;
}	// namespace SoDA
#endif // _SODA_SINGLETONE_H_
