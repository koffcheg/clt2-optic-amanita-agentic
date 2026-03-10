//---------------------------------------------------------------------------

#ifndef mem_storeH
#define mem_storeH
//---------------------------------------------------------------------------
#include <cstdint>

#if defined (__BORLANDC__) && defined (_Windows)
#include "System.hpp"
#endif

class CMemStore {
	uint8_t *buff;            //начало выделенной памяти
	uint8_t *currPtr;         //текущая позиция для записи/чтения
	unsigned int currSize;          //текущий размер выделенной памяти
	unsigned int currFreeLimit;     //остаток свободного места
	bool bIsMemOwner;               //объект может полностью распоряжаться памятью
	//( если bIsMemOwner = true --->при необходимости происходит перераспределение памяти
	//а в деструкторе память освобождается)
public:
	CMemStore();

	CMemStore(const void *ptr, unsigned int size);    //инициализация областью памяти для чтения данных из нее
	~CMemStore();

	int write(const void *ptr, unsigned int size);   //запись данных в объект
	//return :
	//0 - Ok
	//1 - не могу выделить память
	int read(void *ptr, unsigned int size,
			 bool bRealCopy = true);    //чтение данных из объекта. Дополнительно флаг. Если он стоит, происходит реальное копирование, иначе реального чтения нет, а только текущий указатель смещается на нужное количество байтов
	//return :
	//0 - Ok
	//1 - выход за границы выделенной памяти
	int setPosition(unsigned int position);    //установка позиции для чтения/записи
	//return :
	//0 - Ok
	//1 - выход за границы выделенной памяти
	[[nodiscard]] unsigned int size() const { return currSize - currFreeLimit; };//текущий размер (количество данных)
	[[nodiscard]] unsigned int getCurrFreeLimit() const { return currFreeLimit; }

	[[nodiscard]] const uint8_t *data() const { return buff; };       //указатель на начало данных
	[[nodiscard]] uint8_t *getCurrPtr() const { return currPtr; };    //текущий указатель на данные
	[[nodiscard]] unsigned int
	getUseMemSize() const { return currSize; }; //текущий размер выделенной памяти (В режме чтения (при инициализации областью памати для чтения) - это размер инициализированных данных)

	template<typename native_t>
	int write_native(const native_t &arg) {
		return write(&arg, sizeof(native_t));
	}

	template<typename native_t>
	int read_native(native_t &arg) {
		return read(&arg, sizeof(native_t));
	}

	template<typename native_t>
	native_t read_and_ret_native() {
		native_t arg;
		read(&arg, sizeof(native_t));
		return arg;
	}

#if defined (__BORLANDC__) && defined (_Windows)
	void bcbWriteUStringZ(const UnicodeString &str)
	{
		write(str.c_str(), str.Length()*2);
		write_native<uint16_t>(0);
	}

	void bcbReadUString(UnicodeString &str)
	{
		str = String(reinterpret_cast<wchar_t*>(getCurrPtr()));
		read(NULL, str.Length()*2+2, false);
	}
#endif
};

#define WRITE2MEM_STORE(MEM_STORE, ARG2WRITE)    MEM_STORE.write(&ARG2WRITE, sizeof(ARG2WRITE));    //запись
#define RD_FROM_MEM_STORE(MEM_STORE, ARG2READ)    MEM_STORE.read (&ARG2READ,  sizeof(ARG2READ));    //чтение

#endif
