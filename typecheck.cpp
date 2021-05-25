#include "typecheck.hpp"
// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable:
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method:
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class:
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member:
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object:
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch:
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch:
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch:
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch:
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case do_while_predicate_type_mismatch:
      std::cerr << "Predicate of do while loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch:
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch:
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch:
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type:
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class:
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present:
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method:
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature:
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}



// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

void TypeCheck::visitProgramNode(ProgramNode* node) {
  // WRITEME: Replace with code if necessary
  classTable = new ClassTable();
  node->visit_children(this);

  if(classTable->find("Main") == classTable->end())
    typeError(no_main_class);

}

void TypeCheck::visitClassNode(ClassNode* node) {
  ClassInfo clsinfo;
  std::string superClassName, currentClassName;


  // initialization of tables 
  currentMethodTable = new MethodTable();
  currentVariableTable = new VariableTable();

  // checking if superClass exists and inheriting its variables
  clsinfo.superClassName = "";
  clsinfo.membersSize = 0;
  clsinfo.methods = currentMethodTable;
  clsinfo.members = currentVariableTable;
  superClassName = "";
  currentClassName = node->identifier_1->name;
  currentMemberOffset = 0;

  if(node->identifier_2) {
    superClassName = node->identifier_2->name;
    clsinfo.superClassName = superClassName;

    //checking if immediate superclassName exists
    if(classTable->find(superClassName) == classTable->end()) 
      typeError(undefined_class);

    // //inheriting superclass methods and members
    // while(superClassName != "") {
    //   auto superClassInfo = classTable->find(superClassName)->second;
    //   if(superClassInfo.members) {
    //     for(auto memItr = superClassInfo.members->begin(); memItr != superClassInfo.members->end(); memItr++) {
    //       VariableInfo vi = memItr->second;  //unfortunate cant mutate an iterator
    //       vi.offset = currentMemberOffset;

    //       (*currentVariableTable)[memItr->first] = vi;
    //       currentMemberOffset += 4;
    //       clsinfo.membersSize += vi.size; 
    //     }
    //   }
    //   //questionable
    //   if(superClassInfo.methods) {
    //     for(auto methodItr = superClassInfo.methods->begin(); methodItr != superClassInfo.methods->end(); methodItr++) {
    //       (*currentMethodTable)[methodItr->first] = methodItr->second; // do i even need to do a deep copy?? 
    //     }
    //   }
    //   superClassName = superClassInfo.superClassName;
    // }

  }

  (*classTable)[currentClassName] = clsinfo;
  node->visit_children(this);


  for(auto dcl_list = node->declaration_list->begin(); dcl_list != node->declaration_list->end(); dcl_list++) {
    for(auto ids = (*dcl_list)->identifier_list->begin(); ids != (*dcl_list)->identifier_list->end(); dcl_list++) {
      (*currentVariableTable)[(*ids)->name].offset = currentMemberOffset;
      currentMemberOffset += 4;
    }
  }

  if(currentClassName == "Main" && !currentVariableTable->empty()) 
    typeError(main_class_members_present);

}

//validates polymorphism relations on types
bool valPoly(ClassTable* c, std::string child, std::string parent) {
  std::string  superClass;

  superClass = (*c)[child].superClassName;

  while(superClass != parent) {
    if(superClass == "") return false;
    superClass = (*c)[superClass].superClassName;
  }
  return true;
}


void TypeCheck::visitMethodNode(MethodNode* node) {
  // WRITEME: Replace with code if necessary
  MethodInfo mi;

  //is going be filled when you visit children
  currentVariableTable = new VariableTable();
  mi.variables = currentVariableTable;
  mi.parameters = new std::list<CompoundType>();
  mi.localsSize = 0;  // how do i update this??? 

  //since methods body begins here we can set up the offset here
  currentParameterOffset = 12;
  currentLocalOffset = -4;
  
  node->visit_children(this);

  mi.returnType = {node->type->basetype, node->type->objectClassName};
  for(auto prmList = node->parameter_list->begin(); prmList != node->parameter_list->end(); prmList++) {
    mi.parameters->push_back({(*prmList)->type->basetype
                                , (*prmList)->type->objectClassName });
  }
  mi.localsSize = (mi.variables->size()-mi.parameters->size()) * 4;


  (*currentMethodTable)[node->identifier->name] = mi;

  //typechecking to see if return statement matches the return type
  if(node->type->basetype == bt_object) {
    if(classTable->find(node->type->objectClassName) == classTable->end()) 
      typeError(undefined_class);
    
    if(!valPoly(classTable, node->methodbody->objectClassName, node->type->objectClassName))
      typeError(return_type_mismatch);

  } else if(node->type->basetype != node->methodbody->basetype) {
    typeError(return_type_mismatch);
  }

  //checking if constructor have return value
  if(node->identifier->name == currentClassName && node->type->basetype != bt_none) 
    typeError(constructor_returns_type);
  

}



void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  for(auto dcl_list = node->declaration_list->begin(); dcl_list != node->declaration_list->end(); dcl_list++) {
    for(auto ids = (*dcl_list)->identifier_list->begin(); ids != (*dcl_list)->identifier_list->end(); ids++) {
      (*currentVariableTable)[(*ids)->name].offset = currentLocalOffset;
      currentLocalOffset -= 4;
    }
  }

  if(node->returnstatement) {
    node->basetype = node->returnstatement->basetype;
    node->objectClassName = node->returnstatement->objectClassName;
  } else {
    node->basetype = bt_none;
  }
}

void TypeCheck::visitParameterNode(ParameterNode* node) {
  // WRITEME: Replace with code if necessary
  VariableInfo vi; 

  node->visit_children(this);

  vi.type = {node->type->basetype, node->type->objectClassName};
  vi.offset = currentParameterOffset;
  vi.size = 4;
  (*currentVariableTable)[node->identifier->name] = vi;

  currentParameterOffset += 4;
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) {
  // WRITEME: Replace with code if necessary
  VariableInfo vi;

  node->visit_children(this);
  for(auto ids = node->identifier_list->begin(); ids != node->identifier_list->end(); ids++) {
    vi.type = {node->type->basetype, node->type->objectClassName};
    vi.size = 4;
    (*currentVariableTable)[(*ids)->name] = vi;
  }

}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = node->expression->basetype;
  node->objectClassName = node->expression->objectClassName;
}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
  // WRITEME: Replace with code if necessary


}

void TypeCheck::visitCallNode(CallNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitWhileNode(WhileNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitPrintNode(PrintNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitPlusNode(PlusNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitMinusNode(MinusNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitTimesNode(TimesNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitDivideNode(DivideNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitEqualNode(EqualNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitAndNode(AndNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitOrNode(OrNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitNotNode(NotNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitNegationNode(NegationNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitVariableNode(VariableNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitNewNode(NewNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitNoneNode(NoneNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
  // WRITEME: Replace with code if necessary
}


// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}
