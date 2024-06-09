#pragma once

#include "common.h"
#include "formula.h"
#include <set>
#include <optional>
#include <algorithm>
class Cell : public CellInterface {
public:
    Cell() { }
    ~Cell() { }

    void Set(std::string text, SheetInterface& sheet, Position& pos);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override{
        return impl_.get()->GetReferencedCells();
    };
    std::vector<Position> GetDependedCells() const {
        return impl_.get()->GetReferencedCells();
    };

    bool IsReferenced() const;

private:
//можете воспользоваться нашей подсказкой, но это необязательно.


    class Impl {
        public:
        Impl () { }
        virtual ~Impl () = default;
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        
    };

    class EmptyImpl : public Impl {
        public:
        EmptyImpl () { }
        ~EmptyImpl () { }
        void Clear () { };
        Value GetValue() const;
        std::string GetText() const;
        std::vector<Position> GetReferencedCells() const;
    };

    class TextImpl : public Impl {
        public:
        TextImpl (std::string txt) : text_(txt) {}
        ~TextImpl () { }
        void Clear () { };
        Value GetValue() const;
        std::string GetText() const;
        std::vector<Position> GetReferencedCells() const;
        private: 
            std::string text_;
    };

    class FormulaImpl : public Impl {
        public:
        FormulaImpl (std::string txt, SheetInterface& sheet, Position & pos)
            : formula_(ParseFormula(txt))
            , sheet_(sheet)
        { 

            const auto & cells = formula_.get()->GetReferencedCells();
            std::vector<Position> vcells;
            for (const auto & cell : cells) {
                refered_cells_.push_back(cell);
            }
            refered_cells_.erase( std::unique( vcells.begin(), vcells.end() ), vcells.end() );
            // сheck cyclic
            CheckCyclic(this->sheet_, pos);

            for (const auto & cell : refered_cells_) {
                if (sheet_.GetCell(cell) == nullptr) {
                    sheet_.SetCell(cell, {});
                }
            }

        }
        ~FormulaImpl () { }
        void Clear () { };
        Value GetValue() const;
        std::string GetText() const;
        std::vector<Position> GetReferencedCells() const;
        private: 
            std::unique_ptr<FormulaInterface> formula_;
            SheetInterface& sheet_;
            mutable std::optional<FormulaInterface::Value> cache_;
            std::vector<Position> refered_cells_;

            void CheckCyclic(const SheetInterface& sheet, Position pos) {
                std::set<Position> visited;
                visited.insert(pos);
                std::set<Position> stack;
                stack.insert(pos);
                for (const auto & position : refered_cells_) {
                    if (IsCycle(sheet_, visited, stack, position)) {
                        throw CircularDependencyException("Cycle detected");
                    }
                }
            }

            bool IsCycle(const SheetInterface& sheet, std::set<Position>& visited, std::set<Position>& stack, Position pos) const;

    };

    std::unique_ptr<Impl> impl_;
};

