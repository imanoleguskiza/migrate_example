<?php

/**
 * @file
 * Contains IntegrationTestArticlesMigration class.
 */

/**
 * Class EuropaEuInstitutionTypeMigration .
 */
class EuropaEuInstitutionTypeMigration extends EuropaEuAbstractMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct('taxonomy', $arguments);

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationNode::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationTerm('institution_type', array('text_format' => 'full_html')));

    // Entity translation requires that both title fields are mapped.
    $this->addFieldMapping('name', 'subject');
    $this->addFieldMapping('name_field', 'subject');

    // Mapping default language is necessary for correct translation handling.
    $this->addFieldMapping('language', 'default_language');

  }

}
