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
  std::string superClassName;


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
  }

  (*classTable)[currentClassName] = clsinfo;
  node->visit_children(this);

  if(node->declaration_list) {
    for(auto dcl_list = node->declaration_list->begin(); dcl_list != node->declaration_list->end(); dcl_list++) {
      for(auto ids = (*dcl_list)->identifier_list->begin(); ids != (*dcl_list)->identifier_list->end(); ids++) {
        (*currentVariableTable)[(*ids)->name].offset = currentMemberOffset;
        currentMemberOffset += 4;
      }
    }
  }
  

  if(currentClassName == "Main" && !clsinfo.members->empty()) 
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
  VariableTable* before;

  //is going be filled when you visit children
  before = currentVariableTable;
  currentVariableTable = new VariableTable();
  mi.variables = currentVariableTable;
  mi.parameters = NULL;
  mi.localsSize = 0;  // how do i update this??? 

  //since methods body begins here we can set up the offset here
  currentParameterOffset = 12;
  currentLocalOffset = -4;
  
  node->visit_children(this);

  mi.returnType = {node->type->basetype, node->type->objectClassName};
  mi.localsSize = (mi.variables->size()) * 4;

  if(node->parameter_list) {
    mi.parameters = new std::list<CompoundType>();
    for(auto prmList = node->parameter_list->begin(); prmList != node->parameter_list->end(); prmList++) {
      mi.parameters->push_back({(*prmList)->type->basetype
                                  , (*prmList)->type->objectClassName });
      mi.localsSize -=4;
    } 
  }
 ;
  (*currentMethodTable)[node->identifier->name] = mi;

  //typechecking to see if return statement matches the return type
  if(node->type->basetype == bt_object) {
    if(classTable->find(node->type->objectClassName) == classTable->end()) 
      typeError(undefined_class);
    
    if(node->type->basetype != node->methodbody->basetype && !valPoly(classTable, node->methodbody->objectClassName, node->type->objectClassName))
      typeError(return_type_mismatch);

  } else if(node->type->basetype != node->methodbody->basetype) {
    typeError(return_type_mismatch);
  }

  //checking if constructor have return value
  if(node->identifier->name == currentClassName && node->type->basetype != bt_none) 
    typeError(constructor_returns_type);
  
  currentVariableTable = before;

}



void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if(node->declaration_list)
  {
    for(auto dcl_list = node->declaration_list->begin(); dcl_list != node->declaration_list->end(); dcl_list++) {
      for(auto ids = (*dcl_list)->identifier_list->begin(); ids != (*dcl_list)->identifier_list->end(); ids++) {
        (*currentVariableTable)[(*ids)->name].offset = currentLocalOffset;
        currentLocalOffset -= 4;
      }
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


VariableInfo* validateVariable(VariableTable* varTable, ClassTable* classTable, std::string currentClassName, std::string var) {
  VariableTable* clsMembers;

  //checking if the variable exists in current scope
  if(varTable && varTable->find(var) != varTable->end()) return &varTable->at(var);

  //checking if var is a member of this class or superclass
  while(currentClassName != "") {
    clsMembers = (*classTable)[currentClassName].members;
    if(clsMembers && clsMembers->find(var) != clsMembers->end()) return &clsMembers->at(var);
    currentClassName = (*classTable)[currentClassName].superClassName;
  }
  typeError(undefined_variable);
  return NULL;
}

void checkAssignmentMismatch(VariableInfo* lhs, ExpressionNode* rhs, ClassTable* classTable) {
  if(lhs->type.baseType != rhs->basetype) {
      if(lhs->type.baseType != bt_object)
        typeError(assignment_type_mismatch);
      else {
        if(!valPoly(classTable, rhs->objectClassName, lhs->type.objectClassName))
          typeError(assignment_type_mismatch);
      }
    }
}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
  // WRITEME: Replace with code if necessary
  std::string var1, var2, clsName;
  VariableInfo* var1Info, * var2Info;

  node->visit_children(this);


  var1 = node->identifier_1->name;
  var1Info = validateVariable(currentVariableTable, classTable, currentClassName, var1);


  // case1: simple assignment: t_id = expression
  if(!node->identifier_2) {
    checkAssignmentMismatch(var1Info, node->expression, classTable);
  } else { //case 2: t_id.t_id = expression
    var2 = node->identifier_2->name;
    clsName = var1Info-> type.objectClassName; 
    var2Info = validateVariable(NULL, classTable, clsName, var2);
    if(!var2Info) {
      typeError(undefined_member);
    } else {
      checkAssignmentMismatch(var2Info, node->expression, classTable);
    }
  }
}

void TypeCheck::visitCallNode(CallNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_boolean) {
    typeError(if_predicate_type_mismatch);
  }
}

void TypeCheck::visitWhileNode(WhileNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_boolean) {
    typeError(while_predicate_type_mismatch);
  }
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_boolean) {
    typeError(do_while_predicate_type_mismatch);
  }
}

void TypeCheck::visitPrintNode(PrintNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
}

void TypeCheck::visitPlusNode(PlusNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitMinusNode(MinusNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitTimesNode(TimesNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitDivideNode(DivideNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer|| node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer|| node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitEqualNode(EqualNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if( (node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) 
   && (node->expression_1->basetype != bt_integer|| node->expression_2->basetype != bt_integer) ) 
  {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitAndNode(AndNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitOrNode(OrNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitNotNode(NotNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitNegationNode(NegationNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

MethodInfo* validateMethod(ClassTable* classTable, std::string currentClassName, std::string metName) {
  MethodTable* clsMethods;

  //checking if var is a member of this class or superclass
  while(currentClassName != "") {
    clsMethods = (*classTable)[currentClassName].methods;
    if(clsMethods && clsMethods->find(metName) != clsMethods->end()) return &clsMethods->at(metName);
    currentClassName = (*classTable)[currentClassName].superClassName;
  }
  typeError(undefined_method);
  return NULL;
}

void ArgumentMismatch(MethodInfo* info, ClassTable* classTable, std::list<ExpressionNode*>* expression_list) {
  if(!info)  {
      typeError(undefined_method);
    } 

    if(!info->parameters && !expression_list)
      return;
    
    if(!expression_list || !info->parameters) {
      typeError(argument_number_mismatch);
    }

    if(info->parameters->size() !=  expression_list->size()) {
      typeError(argument_number_mismatch);
    } 
    
    auto parItr = info->parameters->begin();
    auto exp_listItr = expression_list->begin();

    for(; parItr != info->parameters->end(); parItr++, exp_listItr++) 
    {
      CompoundType par;
      ExpressionNode* exp;

      par = *parItr;
      exp = *exp_listItr;

      if(exp->basetype != par.baseType) {
        if(exp->basetype != bt_object) {
          typeError(argument_type_mismatch);
        }
        if(!valPoly(classTable, exp->objectClassName, par.objectClassName)) {
            typeError(argument_type_mismatch);
        }
      }
    }
}



void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
  // WRITEME: Replace with code if necessary
  MethodInfo* idInfo1, * idInfo2;
  VariableInfo* varInfo1;

  std::string met1, met2, clsName;

  node->visit_children(this);

  met1 = node->identifier_1->name;

  //case1
  if(!node->identifier_2) {
    idInfo1 = validateMethod(classTable, currentClassName, met1);
    ArgumentMismatch(idInfo1, classTable, node->expression_list);
    node->basetype = idInfo1->returnType.baseType;
    node->objectClassName = idInfo1->returnType.objectClassName;
  } else { //case 2
    met2 = node->identifier_2->name;
    varInfo1 = validateVariable(currentVariableTable, classTable, currentClassName, met1);
    clsName = varInfo1->type.objectClassName; 
    idInfo2 = validateMethod(classTable, clsName, met2);
    ArgumentMismatch(idInfo2, classTable, node->expression_list);
    node->basetype = idInfo2->returnType.baseType;
    node->objectClassName = idInfo2->returnType.objectClassName;
  } 

  
  
}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
  // WRITEME: Replace with code if necessary

  std::string id1, id2, clsName;
  VariableInfo* idInfo1, * idInfo2;

  node->visit_children(this);
  
  id1 = node->identifier_1->name;
  id2 = node->identifier_2->name;

  //check if id1 is a variable of current scope or classMember
  idInfo1 = validateVariable(currentVariableTable, classTable, currentClassName, id1);
  clsName = idInfo1->type.objectClassName;
  idInfo2 = validateVariable(NULL, classTable, clsName, id2);

  node->basetype = idInfo2->type.baseType;
  node->objectClassName = idInfo2->type.objectClassName;

}

void TypeCheck::visitVariableNode(VariableNode* node) {
  // WRITEME: Replace with code if necessary
  std::string id;
  VariableInfo* vInfo;
  node->visit_children(this);

  id = node->identifier->name;
  vInfo = validateVariable(currentVariableTable, classTable, currentClassName, id);

  node->basetype = vInfo->type.baseType;
  node->objectClassName = vInfo->type.objectClassName;
}


void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = bt_integer;
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = bt_boolean;
}

void TypeCheck::visitNewNode(NewNode* node) {
  // WRITEME: Replace with code if necessary
  std::string id;
  MethodTable* metTable;

  id = node->identifier->name;

  if(classTable->find(id) == classTable->end()) {
    typeError(undefined_class);
  }

  //pseudocode
  //if not any constructor : good for case 1 not good for case 2
  //if constructor: check with 0 arguments for case1 , check for case 2 too 
  metTable = classTable->at(id).methods;

  if(node->expression_list) { //second case
    if(!metTable || metTable->find(id) == metTable->end()) {
      typeError(argument_number_mismatch);
    }
    ArgumentMismatch(&metTable->at(id), classTable, node->expression_list);
  } else { //first case
    if(metTable && metTable->find(id) != metTable->end() && metTable->at(id).parameters) {
        typeError(argument_number_mismatch);
    }
  }

  node->basetype = bt_object;
  node->objectClassName = id;
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
}

void TypeCheck::visitNoneNode(NoneNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_none;
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
