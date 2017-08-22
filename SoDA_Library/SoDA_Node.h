#ifndef _SODA_NODE_H_
#define _SODA_NODE_H_

#include <iostream>

using namespace std;

namespace SoDA {

	// When you use "template", don't separate header file and operation file.
	template <typename T>
	class Node {
	public :
		Node()
			: _data(NULL)
			, _front(NULL)
			, _end(NULL)
		{
		}

		Node(T data, Node* front = NULL, Node* end = NULL)
			: _data(NULL)
			, _front(NULL)
			, _end(NULL)
		{
			_data = data;
			_front = front;
			_end = end;
		}

		~Node() { initNode(); }

	private :
		T _data;
		Node<T>* _front;
		Node<T>* _end;

	public :
		void setData(T data) { _data = data; }
		void setFront(Node<T>* front) { _front = front; }
		void setEnd(Node<T>* end) { _end = end; }

		T getData() { return _data; }
		Node<T>* getFront() { return _front; }
		Node<T>* getEnd() { return _end; }

	private :
		void initNode()
		{
			_data = NULL;
			_front = NULL;
			_end = NULL;
		}
	}; // class Node
}	// namespace SoDA


#endif // _SODA_NODE_H_