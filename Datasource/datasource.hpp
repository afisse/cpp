#ifndef DATASOURCE_H
#define DATASOURCE_H

class Table;

class TableParameter {};

class DatasourceParameter {};

class Datasource {
private:
  DatasourceParameter datasourceParameter;
public:
  Table createTable (TableParameter);
};
 
class Table {
private:
  TableParameter tableParameter;
  Datasource datasource;
public:
  Table(TableParameter);
  Datasource getDatasource ();
};

class Comprehension {
private:
  Table table;
public:
  Table getTable ();
};

class Result {};

class ComprehensionEngine {
public:
  Result run (Comprehension);
};

#endif /* DATASOURCE_H */
