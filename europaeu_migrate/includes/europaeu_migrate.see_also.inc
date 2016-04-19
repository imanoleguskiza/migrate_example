<?php

/**
 * @file
 * Contains IntegrationTestArticlesMigration class.
 */

/**
 * Class EuropaEuCommonPageMigration .
 */
class EuropaEuSeeAlsoMigration extends EuropaEuAbstractMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct('see_also', $arguments);

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationNode::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationNode('see_also', array('text_format' => 'full_html')));

    // Entity translation requires that both title fields are mapped.
    $this->addFieldMapping('title', 'document_title');
    $this->addFieldMapping('title_field', 'document_title');
    $this->addFieldMapping('field_see_also_link', 'link');
    $this->addFieldMapping('field_see_also_link:title', 'title');

    // Mapping default language is necessary for correct translation handling.
    $this->addFieldMapping('language', 'default_language');

    $this->addFieldMapping('promote')->defaultValue(FALSE);
    $this->addFieldMapping('status')->defaultValue(NODE_PUBLISHED);
  }

}
