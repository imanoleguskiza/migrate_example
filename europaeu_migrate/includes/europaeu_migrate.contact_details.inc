<?php

/**
 * @file
 * Contains IntegrationTestArticlesMigration class.
 */

/**
 * Class EuropaEuContactDetailsMigration .
 */
class EuropaEuContactDetailsMigration extends EuropaEuAbstractMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct('contact', $arguments);

    // Requires taxonomy terms migration to run before content migration.
    $this->setHardDependencies(array());

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationNode::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationNode('contact_details', array('text_format' => 'full_html')));

    // Entity translation requires that both title fields are mapped.
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('title_field', 'title');
    $this->addFieldMapping('field_contact_address', 'adress');
    $this->addFieldMapping('field_contact_address:format')->defaultValue('full_html');
    $this->addFieldMapping('field_contact_fax', 'fax');
    $this->addFieldMapping('field_contact_links', 'link_url');
    $this->addFieldMapping('field_contact_links:title', 'link_label');
    $this->addFieldMapping('field_contact_logo', 'logo');
    $this->addFieldMapping('field_contact_logo:urlencode')->defaultValue(0);

    $this->addFieldMapping('field_contact_social_networks', 'social_url');
    $this->addFieldMapping('field_contact_social_networks:title', 'social_label');
    $this->addFieldMapping('field_contact_tel', 'tel');
    $this->addFieldMapping('field_contact_visit', 'visit');

    // Mapping default language is necessary for correct translation handling.
    $this->addFieldMapping('language', 'default_language');

    $this->addFieldMapping('promote')->defaultValue(FALSE);
    $this->addFieldMapping('status')->defaultValue(NODE_PUBLISHED);
  }

}
