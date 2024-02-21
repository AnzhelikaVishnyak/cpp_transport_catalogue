#include "json_builder.h"

namespace json {

	// -------------- BaseContext --------------

	Builder::BaseContext::BaseContext(Builder& builder) : builder_(builder) {}

	Node Builder::BaseContext::Build()
	{
		return builder_.Build();
	}

	Builder::DictValueContext Builder::BaseContext::Key(std::string key)
	{
		return builder_.Key(std::move(key));
	}

	Builder::BaseContext Builder::BaseContext::Value(json::Node value)
	{
		return builder_.Value(std::move(value));
	}

	Builder::DictItemContext Builder::BaseContext::StartDict()
	{
		return builder_.StartDict();
	}

	Builder::ArrItemContext Builder::BaseContext::StartArray()
	{
		return builder_.StartArray();
	}

	Builder::BaseContext Builder::BaseContext::EndDict()
	{
		return builder_.EndDict();
	}

	Builder::BaseContext Builder::BaseContext::EndArray()
	{
		return builder_.EndArray();
	}

	// -------------- DictItemContext --------------

	Builder::DictItemContext::DictItemContext(BaseContext base) : BaseContext(base) {}

	// -------------- DictValueContext --------------

	Builder::DictValueContext::DictValueContext(BaseContext base) : BaseContext(base) {}

	Builder::DictItemContext Builder::DictValueContext::Value(json::Node value)
	{
		return BaseContext::Value(std::move(value));
	}

	// -------------- ArrItemContext --------------

	Builder::ArrItemContext::ArrItemContext(BaseContext base) : BaseContext(base) {}

	Builder::ArrItemContext Builder::ArrItemContext::Value(json::Node value)
	{
		return BaseContext::Value(std::move(value));
	}


	// -------------- Builder --------------


	Builder::DictValueContext Builder::Key(std::string key)
	{
		if (!nodes_stack_.empty()) {
			if (!nodes_stack_.back()->IsDict()) {
				throw std::logic_error("Invalid Key");
			}

			std::get<Dict>(nodes_stack_.back()->GetValue())[key];
			key_ = key;

			return BaseContext{ *this };
		}
		throw std::logic_error("Invalid Key");
	}

	Builder::BaseContext Builder::Value(json::Node value)
	{
		return AddNode(std::move(value), false);
	}

	Builder::DictItemContext Builder::StartDict()
	{
		return AddNode(Dict{}, true);
	}

	Builder::ArrItemContext Builder::StartArray()
	{
		return AddNode(Array{}, true);
	}

	Builder::BaseContext Builder::EndDict()
	{
		if (!nodes_stack_.empty()) {
			if (!nodes_stack_.back()->IsDict()) {
				throw std::logic_error("Invalid EndDict");
			}

			nodes_stack_.pop_back();
			return BaseContext{ *this };
		}

		throw std::logic_error("Invalid EndDict");

	}

	Builder::BaseContext Builder::EndArray()
	{
		if (!nodes_stack_.empty()) {
			if (!nodes_stack_.back()->IsArray()) {
				throw std::logic_error("Invalid EndArray");
			}
			nodes_stack_.pop_back();
			return BaseContext{ *this };
		}

		throw std::logic_error("Invalid EndArray");
	}

	Node Builder::Build()
	{
		if (!(nodes_stack_.empty()) || root_ == nullptr) {
			throw std::logic_error("Invalid JSON");
		}

		return root_;
	}

	Builder::BaseContext Builder::AddNode(json::Node value, bool push_to_stack)
	{
		Node* insertedValue = nullptr;

		if (root_ == nullptr) {
			root_.GetValue() = value;
			insertedValue = &root_;
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			insertedValue = &std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(value));
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			std::get<Dict>(nodes_stack_.back()->GetValue())[key_].GetValue() = std::move(value);
			insertedValue = &(std::get<Dict>(nodes_stack_.back()->GetValue())[key_]);
		}
		else {
			throw std::logic_error("Invalid Start Container or Value");
		}

		if (push_to_stack)
		{		
			nodes_stack_.emplace_back(insertedValue);
		}

		return BaseContext{ *this };
	}

}