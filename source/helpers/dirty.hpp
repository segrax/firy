namespace firy {
	namespace helpers {
		class cDirty {
		public:
			cDirty() : mDirty(false) {}

			virtual inline bool isDirty() const { return mDirty; }

			/**
			 * Set dirty state, return state
			 */
			virtual inline bool dirty(const bool pVal = true) { mDirty = pVal; return mDirty; }

		private:
			volatile bool mDirty;
		};
	}
}

