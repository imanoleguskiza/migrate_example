<?php
/**
 * @file
 * Contains EuropaEuCommonPageMigration class.
 */

/**
 * Class EuropaEuCommonPageMigration.
 */
class EuropaEuBeanMigration extends EuropaEuAbstractMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct('bean', $arguments);

    // Requires taxonomy terms migration to run before content migration.
    $this->setHardDependencies(array());

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationBean::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationBean('webtools', array('text_format' => 'full_html')));

    // Entity translation requires that both title fields are mapped.
    $this->addFieldMapping('label', 'title');
    $this->addFieldMapping('title')->defaultValue('<none>');
    $this->addFieldMapping('field_json_object', 'json_object');

    // Mapping default language is necessary for correct translation handling.
    $this->addFieldMapping('language', 'default_language');
    $this->addFieldMapping('type')->defaultValue('webtools');
  }

  /**
   * Implements prepareRow().
   */
  public function prepareRow(&$row) {
    $dummy_domain = "http://localhost/platform";
    global $base_url;
    $current_domain = $base_url;
    $row->json_object = str_replace($dummy_domain, $current_domain, $row->json_object);
  }

}
