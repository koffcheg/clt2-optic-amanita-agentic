#include "mem_store.h"
#include <cstring>

CMemStore::CMemStore() :
		buff(nullptr),
		currPtr(nullptr),
		currSize(0),
		currFreeLimit(0),
		bIsMemOwner(true)  //объект может распоряжаться памятью по своему усмотрению
{}

CMemStore::CMemStore(const void *ptr, unsigned int size) :    //инициализация областью памяти для чтения данных из нее
		buff(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(ptr))),
		currPtr(buff),
		currSize(size),
		currFreeLimit(size),
		bIsMemOwner(false)  //объекту нельзя распоряжаться памятью - работаем в режиме чтения
{
}

//------------------------------
CMemStore::~CMemStore() {
	if (bIsMemOwner && (buff != nullptr)) {
		delete[] buff;
		buff = nullptr;
	}
}

//-------запись информации в объект------------
int CMemStore::write(const void *ptr, unsigned int _size) {
	if (bIsMemOwner && buff == nullptr) //еще ничего не записывали - выделяем память
	{
		currSize = 1024;
		buff = new uint8_t[currSize]; //по-умолчанию 1кб
		currPtr = buff;
		currFreeLimit = currSize;
	}
	if (currFreeLimit < _size)    //пямяти нехватает
	{
		if (!bIsMemOwner)
			return 1;
		unsigned int newSize = currSize;    //новый размер
		unsigned int fillPart = currSize - currFreeLimit;   //столько сейчас занято
		unsigned int newFreeLim;   //столько будет свободно после переаспределения
		do {
			newSize *= 2;    //увеличиваем память в 2 раза
			newFreeLim = newSize - fillPart;    //новый объем свободного места
		} while (newFreeLim < _size);
		auto *newBuff = new uint8_t[newSize];  //новый буффер
		memcpy(newBuff, buff, fillPart);    //перенос данных в новый буфер
		delete[] buff;  //старый буфер освобождаем
		buff = newBuff;                 //перенастраиваем переменные -> начало выделенной памяти
		currPtr = buff + fillPart;      //перенастраиваем переменные -> начало памяти куда нужно записывать данные
		currSize = newSize;             //перенастраиваем переменные -> общий размер выделенной памяти
		currFreeLimit = currSize - fillPart;    //перенастраиваем переменные -> количество свободной памяти
	}
	memcpy(currPtr, ptr, _size);    //перенос данных
	currPtr += _size;           //перенастраиваем переменные -> начало памяти куда нужно записывать данные
	currFreeLimit -= _size;      //перенастраиваем переменные -> количество свободной памяти
	return 0;
}

//------------------чтение данных из объекта--------------------------------
int CMemStore::read(void *ptr, unsigned int _size,
					bool bRealCopy) {    //Дополнительно флаг. Если он стоит, происходит реальное копирование, иначе реального чтения нет, а только текущий указатель смещается на нужное количество байтов
	if (bRealCopy)    //если реальное чтение
		memcpy(ptr, currPtr, _size);    //перенос данных
	if (currFreeLimit >= _size)           //пока еще находимся в пределах выделенной памяти
	{
		currPtr += _size;           //перенастраиваем переменные -> начало памяти откуда можно читать(записывать)
		currFreeLimit -= _size;      //перенастраиваем переменные -> количество свободной памяти
		return 0;
	} else   //чтение произошло за пределами выделеной памяти
	{
		currPtr = buff + currSize;  //перенастраиваем переменные -> указатель для текущих операций в конец памяти
		currFreeLimit = 0;          //перенастраиваем переменные -> свободной памяти НЕТ
		return 1;
	}
}

//-----------установка позиции для чтения/записи-----------------------
int CMemStore::setPosition(unsigned int position) {
	if (position < currSize) {
		currPtr = buff + position;  //перенастраиваем переменные -> начало памяти откуда можно читать(записывать)
		currFreeLimit = currSize - position;          //перенастраиваем переменные
		return 0;
	} else {
		currPtr = buff + currSize;  //перенастраиваем переменные -> указатель для текущих операций в конец памяти
		currFreeLimit = 0;          //перенастраиваем переменные -> свободной памяти НЕТ
		return 1;
	}
}



