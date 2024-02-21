#pragma once

#include "json.h"

namespace json {

	class Builder
	{
	private:
		class BaseContext;
		class DictValueContext;
		class DictItemContext;
		class ArrItemContext;

	public:

		Builder() = default;
		~Builder() = default;

		DictValueContext Key(std::string key);
		BaseContext Value(json::Node value);
		DictItemContext StartDict();
		ArrItemContext StartArray();
		BaseContext EndDict();
		BaseContext EndArray();

		Node Build();
	private:
		Node root_ = nullptr;
		std::vector<Node*> nodes_stack_;
		std::string key_;

	private:
		BaseContext AddNode(json::Node value, bool push_to_stack);

		class BaseContext
		{
		public:
			explicit BaseContext(Builder& builder);

			Node Build();
			DictValueContext Key(std::string key);
			BaseContext Value(json::Node value);
			DictItemContext StartDict();
			ArrItemContext StartArray();
			BaseContext EndDict();
			BaseContext EndArray();
		private:
			Builder& builder_;
		};

		class DictItemContext : public BaseContext
		{
		public:
			DictItemContext() = delete;
			DictItemContext(BaseContext base);

			Node Build() = delete;
			BaseContext Value(json::Node value) = delete;
			DictItemContext StartDict() = delete;
			ArrItemContext StartArray() = delete;
			BaseContext EndArray() = delete;
		};

		class DictValueContext : public BaseContext
		{
		public:
			DictValueContext() = delete;
			DictValueContext(BaseContext base);

			DictItemContext Value(json::Node value);

			Node Build() = delete;
			DictValueContext Key(std::string key) = delete;
			BaseContext EndDict() = delete;
			BaseContext EndArray() = delete;
		};

		class ArrItemContext : public BaseContext
		{
		public:
			ArrItemContext() = delete;
			ArrItemContext(BaseContext base);

			ArrItemContext Value(json::Node value);

			Node Build() = delete;
			DictValueContext Key(std::string key) = delete;
			BaseContext EndDict() = delete;
		};
	
	};
}