class Sketch {
public:
	virtual uint64_t getMemoryAccessCounter() const = 0;
	virtual void Insert(const char* str)=0;
	virtual int Query(const char* str)const=0;
	virtual long double Join(Sketch* other)=0;
	virtual bool CheckHeavy(const char* str){
		return 0;
	}
};