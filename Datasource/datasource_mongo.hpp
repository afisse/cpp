#ifndef DATASOURCE_MONGO_H
#define DATASOURCE_MONGO_H

#include "datasource.hpp"

class TableParameterMongo : TableParameter {
  
};

class DatasourceParameterMongo : DatasourceParameter {

};

class TableMongo : Table {
public:
  TableMongo (TableParameterMongo);
};

class DatasourceMongo : Datasource {

};

#endif /* DATASOURCE_MONGO_H */
