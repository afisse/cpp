#include "datasource_mongo.hpp"

TableMongo::TableMongo (TableParameterMongo tmp) {
  tableParameter = tmp;
}

DatasourceMongo::createTable (TableParameterMongo tpm) {
  return TableMongo (tmp);
}
